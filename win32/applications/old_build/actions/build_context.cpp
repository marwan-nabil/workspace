#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\path_handling.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\build.h"

void AddCompilerFlags(build_context *BuildContext, const char *Flags)
{
    StringCchCatA
    (
        BuildContext->CompilationInfo.CompilerFlags,
        ArrayCount(BuildContext->CompilationInfo.CompilerFlags),
        Flags
    );
    StringCchCatA
    (
        BuildContext->CompilationInfo.CompilerFlags,
        ArrayCount(BuildContext->CompilationInfo.CompilerFlags),
        " "
    );
}

void SetCompilerIncludePath(build_context *BuildContext, const char *IncludePath)
{
    StringCchCatA
    (
        BuildContext->CompilationInfo.CompilerIncludePath,
        ArrayCount(BuildContext->CompilationInfo.CompilerIncludePath),
        BuildContext->EnvironmentInfo.WorkspaceDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->CompilationInfo.CompilerIncludePath,
        ArrayCount(BuildContext->CompilationInfo.CompilerIncludePath),
        IncludePath
    );
}

void AddCompilerSourceFile(build_context *BuildContext, const char *SourceFile)
{
    PushStringNode
    (
        &BuildContext->CompilationInfo.Sources,
        BuildContext->EnvironmentInfo.WorkspaceDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->CompilationInfo.Sources->String,
        ArrayCount(BuildContext->CompilationInfo.Sources->String),
        SourceFile
    );
}

void SetCompilerOutputObject(build_context *BuildContext, const char *ObjectFile)
{
    StringCchCatA
    (
        BuildContext->CompilationInfo.OutputObjectPath,
        ArrayCount(BuildContext->CompilationInfo.OutputObjectPath),
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->CompilationInfo.OutputObjectPath,
        ArrayCount(BuildContext->CompilationInfo.OutputObjectPath),
        ObjectFile
    );
}

void AddLinkerFlags(build_context *BuildContext, const char *Flags)
{
    StringCchCatA
    (
        BuildContext->LinkingInfo.LinkerFlags,
        ArrayCount(BuildContext->LinkingInfo.LinkerFlags),
        Flags
    );
    StringCchCatA
    (
        BuildContext->LinkingInfo.LinkerFlags,
        ArrayCount(BuildContext->LinkingInfo.LinkerFlags),
        " "
    );
}

void SetLinkerScriptPath(build_context *BuildContext, const char *LinkerScriptFile)
{
    StringCchCatA
    (
        BuildContext->LinkingInfo.LinkerScriptPath,
        ArrayCount(BuildContext->LinkingInfo.LinkerScriptPath),
        BuildContext->EnvironmentInfo.WorkspaceDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->LinkingInfo.LinkerScriptPath,
        ArrayCount(BuildContext->LinkingInfo.LinkerScriptPath),
        LinkerScriptFile
    );
}

void AddLinkerInputFile(build_context *BuildContext, char *LinkerInputFile)
{
    PushStringNode
    (
        &BuildContext->LinkingInfo.LinkerInputs,
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->LinkingInfo.LinkerInputs->String,
        ArrayCount(BuildContext->LinkingInfo.LinkerInputs->String),
        LinkerInputFile
    );
}

void SetLinkerOutputBinary(build_context *BuildContext, char *OutputBinaryPath)
{
    StringCchCatA
    (
        BuildContext->LinkingInfo.OutputBinaryPath,
        ArrayCount(BuildContext->LinkingInfo.OutputBinaryPath),
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath
    );
    StringCchCatA
    (
        BuildContext->LinkingInfo.OutputBinaryPath,
        ArrayCount(BuildContext->LinkingInfo.OutputBinaryPath),
        OutputBinaryPath
    );
}

void PushSubTarget(build_context *BuildContext, const char *SubTargetRelativePath)
{
    StringCchCatA
    (
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath,
        ArrayCount(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath),
        "\\"
    );
    StringCchCatA
    (
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath,
        ArrayCount(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath),
        SubTargetRelativePath
    );

    CreateDirectoryA(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath, NULL);
    SetCurrentDirectoryA(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath);
}

void PopSubTarget(build_context *BuildContext)
{
    RemoveLastSegmentFromPath(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath, FALSE, '\\');
    SetCurrentDirectoryA(BuildContext->EnvironmentInfo.TargetOutputDirectoryPath);
}

void ClearBuildContext(build_context *BuildContext)
{
    FreeStringList(BuildContext->CompilationInfo.Sources);
    FreeStringList(BuildContext->LinkingInfo.LinkerInputs);
    BuildContext->CompilationInfo = {};
    BuildContext->LinkingInfo = {};
}