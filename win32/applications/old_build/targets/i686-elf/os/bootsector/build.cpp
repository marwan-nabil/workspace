#include <Windows.h>
#include <stdint.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\nasm.h"

b32 BuildBootSectorImage(build_context *BuildContext)
{
    PushSubTarget(BuildContext, "bootsector");
    AddCompilerSourceFile(BuildContext, "\\i686-elf\\os\\bootsector\\entry.s");
    AddCompilerFlags(BuildContext, "-f bin -lbootsector.lst");
    SetCompilerIncludePath(BuildContext, "\\");
    SetCompilerOutputObject(BuildContext, "\\bootsector.img");
    b32 BuildSuccess = AssembleWithNasm(BuildContext);
    ClearBuildContext(BuildContext);
    PopSubTarget(BuildContext);
    return BuildSuccess;
}