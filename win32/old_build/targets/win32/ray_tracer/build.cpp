#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"
#include "win32\tools\build\build.h"

b32 BuildRayTracer(build_context *BuildContext)
{
    if (BuildContext->EnvironmentInfo.argc < 3)
    {
        ConsolePrintColored("ERROR: invalid number of arguments for build ray_tracer ...\n", FOREGROUND_RED);
        DisplayHelp();
        return FALSE;
    }

    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\ray_tracer\\main.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\ray_tracer\\brdf.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\file_system\\files.cpp");

    AddCompilerFlags(BuildContext, "/nologo /Z7 /FC /Oi /O2 /GR- /EHa- /MTd /fp:fast /fp:except-");
    AddCompilerFlags(BuildContext, "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018");
    AddCompilerFlags(BuildContext, "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, "/subsystem:console /incremental:no /opt:ref user32.lib gdi32.lib");

    if (strcmp(BuildContext->EnvironmentInfo.argv[2], "1_lane") == 0)
    {
        AddCompilerFlags(BuildContext, "/DSIMD_NUMBEROF_LANES=1");
        SetLinkerOutputBinary(BuildContext, "\\ray_tracer_1.exe");
    }
    else if (strcmp(BuildContext->EnvironmentInfo.argv[2], "4_lanes") == 0)
    {
        AddCompilerFlags(BuildContext, "/DSIMD_NUMBEROF_LANES=4");
        SetLinkerOutputBinary(BuildContext, "\\ray_tracer_4.exe");
    }
    else if (strcmp(BuildContext->EnvironmentInfo.argv[2], "8_lanes") == 0)
    {
        AddCompilerFlags(BuildContext, "/DSIMD_NUMBEROF_LANES=8");
        SetLinkerOutputBinary(BuildContext, "\\ray_tracer_8.exe");
    }
    else
    {
        ConsoleSwitchColor(FOREGROUND_RED);
        printf("ERROR: invalid argument \"%s\" for build ray_tracer ...\n", BuildContext->EnvironmentInfo.argv[3]);
        ConsoleResetColor();
        DisplayHelp();
        return FALSE;
    }

    b32 BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}