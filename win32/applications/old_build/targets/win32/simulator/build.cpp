#include <Windows.h>
#include <stdint.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"

b32 BuildSimulator(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\simulator\\logic_gates.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\simulator\\main.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\simulator\\rendering.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\applications\\simulator\\simulation.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\shell\\windows.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\rasterizer\\rasterizer.cpp");

    AddCompilerFlags(BuildContext, "/nologo /Z7 /FC /Oi /Od /GR- /EHa- /MTd /fp:fast /fp:except-");
    AddCompilerFlags(BuildContext, "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018");
    AddCompilerFlags(BuildContext, "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, "/subsystem:windows /incremental:no /opt:ref user32.lib gdi32.lib winmm.lib");
    SetLinkerOutputBinary(BuildContext, "\\simulator.exe");
    b32 BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}