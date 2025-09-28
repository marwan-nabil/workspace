#include <Windows.h>
#include <stdint.h>
#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\msvc.h"

b32 BuildFat12Tests(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\win32\\tests\\fat12_tests\\test.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\shell\\console.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\strings\\path_handling.cpp");
    AddCompilerSourceFile(BuildContext, "\\win32\\shared\\file_system\\fat12\\fat12*.cpp");

    AddCompilerFlags(BuildContext, "/nologo /Z7 /FC /Od /GR- /EHa- /MTd /fp:fast /fp:except-");
    AddCompilerFlags(BuildContext, "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 /wd4127");
    AddCompilerFlags(BuildContext, "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS");
    SetCompilerIncludePath(BuildContext, "\\");
    AddLinkerFlags(BuildContext, "/subsystem:console /incremental:no /opt:ref user32.lib");
    SetLinkerOutputBinary(BuildContext, "\\fat12_tests.exe");

    b32 BuildSuccess = CompileAndLinkWithMSVC(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}