#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"
#include "win32\tools\build\build.h"

b32 BuildImguiDemo(build_context *BuildContext)
{
    if (BuildContext->EnvironmentInfo.argc < 3)
    {
        ConsolePrintColored
        (
            "ERROR: invalid number of arguments for build imgui_demo ...\n",
            FOREGROUND_RED
        );
        DisplayHelp();
        return FALSE;
    }

    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\imgui\\imgui*.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\shell\\dpi.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\system\\version.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\demos\\imgui\\win32_backend.cpp");

    AddCompilerFlags(BuildContext, "/nologo /Zi /MD /utf-8");
    AddCompilerFlags(BuildContext, "/DUNICODE /D_UNICODE /DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS");
    AddCompilerFlags(BuildContext, "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018");
    SetCompilerIncludePath(BuildContext, "\\");

    AddLinkerFlags(BuildContext, "user32.lib Gdi32.lib dwmapi.lib");

    if (strcmp(BuildContext->EnvironmentInfo.argv[2], "opengl2") == 0)
    {
        AddCompilerSourceFile(BuildContext, "\\win32\\demos\\imgui\\main_opengl2.cpp");
        AddCompilerSourceFile(BuildContext, "\\win32\\demos\\imgui\\opengl2_backend.cpp");
        AddLinkerFlags(BuildContext, "opengl32.lib");
        SetLinkerOutputBinary(BuildContext, "\\imgui_demo_opengl2.exe");
    }
    else if (strcmp(BuildContext->EnvironmentInfo.argv[2], "dx11") == 0)
    {
        AddCompilerSourceFile(BuildContext, "\\win32\\demos\\imgui\\main_dx11.cpp");
        AddCompilerSourceFile(BuildContext, "\\win32\\demos\\imgui\\dx11_backend.cpp");
        AddLinkerFlags(BuildContext, "d3d11.lib d3dcompiler.lib");
        SetLinkerOutputBinary(BuildContext, "\\imgui_demo_dx11.exe");
    }
    else
    {
        ConsoleSwitchColor(FOREGROUND_RED);
        printf("ERROR: invalid argument \"%s\" for build imgui_demo ...\n", BuildContext->EnvironmentInfo.argv[2]);
        ConsoleResetColor();
        DisplayHelp();
        return FALSE;
    }

    b32 BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}