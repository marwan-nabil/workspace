#include <Windows.h>
#include <stdint.h>
#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\strings\string_list.h"

#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\actions\msvc.h"

b32 BuildBuildTests(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\tests\\build_tests\\build_tests.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\libraries\\system\\processes.cpp");
    AddCompilerSourceFile(BuildContext, "\\sources\\win32\\libraries\\shell\\console.cpp");

    AddCompilerFlags(BuildContext, "/nologo /FC /O2 /Oi /GR- /EHa- /MTd /fp:fast /fp:except-");
    AddCompilerFlags(BuildContext, "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 /wd4127");
    AddCompilerFlags(BuildContext, "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, "/subsystem:console /incremental:no /opt:ref user32.lib");
    SetLinkerOutputBinary(BuildContext, "\\build_tests.exe");
    b32 BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}