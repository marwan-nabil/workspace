#include <Windows.h>
#include <stdint.h>
#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\strings\string_list.h"

#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\actions\nasm.h"

b32 BuildBootSectorImage(build_context *BuildContext)
{
    PushSubTarget(BuildContext, "bootsector");
    AddCompilerSourceFile(BuildContext, "\\sources\\i686-elf\\os\\bootsector\\entry.s");
    AddCompilerFlags(BuildContext, "-f bin -lbootsector.lst");
    SetCompilerIncludePath(BuildContext, "\\");
    SetCompilerOutputObject(BuildContext, "\\bootsector.img");
    b32 BuildSuccess = AssembleWithNasm(BuildContext);
    ClearBuildContext(BuildContext);
    PopSubTarget(BuildContext);
    return BuildSuccess;
}