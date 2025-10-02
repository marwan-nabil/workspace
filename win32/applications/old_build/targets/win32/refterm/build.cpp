#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\strings\path_handling.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"

static b32 BuildShaders(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\refterm\\refterm.hlsl");
    AddCompilerFlags(BuildContext, "/nologo /T cs_5_0 /E ComputeMain /O3 /WX /Fh refterm_cs.h /Vn ReftermCSShaderBytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv");
    b32 BuildSuccess = CompileShader2(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\refterm\\refterm.hlsl");
    AddCompilerFlags(BuildContext, "/nologo /T ps_5_0 /E PixelMain /O3 /WX /Fh refterm_ps.h /Vn ReftermPSShaderBytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv");
    BuildSuccess = CompileShader2(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\refterm\\refterm.hlsl");
    AddCompilerFlags(BuildContext, "/nologo /T vs_5_0 /E VertexMain /O3 /WX /Fh refterm_vs.h /Vn ReftermVSShaderBytes /Qstrip_reflect /Qstrip_debug /Qstrip_priv");
    BuildSuccess = CompileShader2(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    char *FilesToMove[3] =
    {
        "refterm_cs.h",
        "refterm_vs.h",
        "refterm_ps.h",
    };

    char SourceFileName[1024] = {};
    StringCchCat(SourceFileName, 1024, BuildContext->EnvironmentInfo.TargetOutputDirectoryPath);
    StringCchCat(SourceFileName, 1024, "\\");

    char DestinationFileName[1024] = {};
    StringCchCat(DestinationFileName, 1024, BuildContext->EnvironmentInfo.WorkspaceDirectoryPath);
    StringCchCat(DestinationFileName, 1024, "\\win32\\demos\\refterm\\");

    for (u8 FileIndex = 0; FileIndex < ArrayCount(FilesToMove); FileIndex++)
    {
        // StringCchCat(SourceFileName, 1024, "\\");
        StringCchCat(SourceFileName, 1024, FilesToMove[FileIndex]);
        // StringCchCat(DestinationFileName, 1024, "\\");
        StringCchCat(DestinationFileName, 1024, FilesToMove[FileIndex]);
        MoveFile(SourceFileName, DestinationFileName);
        RemoveLastSegmentFromPath(SourceFileName, TRUE, '\\');
        RemoveLastSegmentFromPath(DestinationFileName, TRUE, '\\');
    }

    return BuildSuccess;
}

b32 BuildRefTerm(build_context *BuildContext)
{
    b32 BuildSuccess = BuildShaders(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }

    char *SharedCompilerFlags = "/nologo /W3 /Z7 /GS- /Gs999999";
    char *SharedLinkerFlags = "/incremental:no /opt:icf /opt:ref";
    char *SharedSourceFiles[] =
    {
        "\\win32\\demos\\refterm\\refterm.c",
        "\\win32\\demos\\refterm\\refterm_example_dwrite.cpp"
    };

    AddCompilerSourceFile(BuildContext, SharedSourceFiles[0]);
    AddCompilerSourceFile(BuildContext, SharedSourceFiles[1]);
    AddCompilerFlags(BuildContext, SharedCompilerFlags);
    AddCompilerFlags(BuildContext, "/D_DEBUG /Od");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, SharedLinkerFlags);
    AddLinkerFlags(BuildContext, "/subsystem:windows");
    SetLinkerOutputBinary(BuildContext, "\\refterm_debug_msvc.exe");
    BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    AddCompilerSourceFile(BuildContext, SharedSourceFiles[0]);
    AddCompilerSourceFile(BuildContext, SharedSourceFiles[1]);
    AddCompilerFlags(BuildContext, SharedCompilerFlags);
    AddCompilerFlags(BuildContext, "/O2");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, SharedLinkerFlags);
    AddLinkerFlags(BuildContext, "/subsystem:windows");
    SetLinkerOutputBinary(BuildContext, "\\refterm_release_msvc.exe");
    BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\refterm\\splat.cpp");
    AddCompilerFlags(BuildContext, SharedCompilerFlags);
    AddCompilerFlags(BuildContext, "/O2");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, SharedLinkerFlags);
    AddLinkerFlags(BuildContext, "/subsystem:console");
    SetLinkerOutputBinary(BuildContext, "\\splat.exe");
    BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\refterm\\splat2.cpp");
    AddCompilerFlags(BuildContext, SharedCompilerFlags);
    AddCompilerFlags(BuildContext, "/O2");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, SharedLinkerFlags);
    AddLinkerFlags(BuildContext, "/subsystem:console");
    SetLinkerOutputBinary(BuildContext, "\\splat2.exe");
    BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    if (!BuildSuccess)
    {
        return FALSE;
    }
    ClearBuildContext(BuildContext);

    return BuildSuccess;
}