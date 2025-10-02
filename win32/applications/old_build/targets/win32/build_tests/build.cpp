#include <Windows.h>
#include <stdint.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"

b32 BuildBuildTests(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\win32\\tests\\build_tests\\build_tests.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\system\\processes.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\shell\\console.cpp");

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