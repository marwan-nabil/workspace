#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\iverilog.h"

b32 BuildVerilogDemo(build_context *BuildContext)
{
    AddCompilerSourceFile(BuildContext, "\\verilog\\verilog_demo\\testbench.v");
    SetCompilerIncludePath(BuildContext, "\\verilog\\verilog_demo");
    SetCompilerOutputObject(BuildContext, "\\testbench.vvp");
    b32 BuildSuccess = CompileWithIVerilog(BuildContext);
    ClearBuildContext(BuildContext);
    return BuildSuccess;
}