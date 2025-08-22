#include <Windows.h>
#include <stdint.h>
#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\strings\string_list.h"

#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\actions\msvc.h"

b32 BuildSimulator(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\applications\\simulator\\logic_gates.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\applications\\simulator\\main.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\applications\\simulator\\rendering.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\applications\\simulator\\simulation.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\libraries\\shell\\windows.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\libraries\\rasterizer\\rasterizer.cpp");

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