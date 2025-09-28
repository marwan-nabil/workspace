#pragma once

#include "win32\shared\base_types.h"
#include "win32\build\actions\build_context.h"

b32 CompileShader(build_context *BuildContext);
b32 CompileShader2(build_context *BuildContext);
b32 CompileWithMSVC(build_context *BuildContext);
b32 LinkWithMSVC(build_context *BuildContext);
b32 CompileAndLinkWithMSVC(build_context *BuildContext);