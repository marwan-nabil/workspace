#pragma once

#include "win32\shared\base_types.h"
#include "win32\build\actions\build_context.h"

b32 CompileWithGCC(build_context *BuildContext);
b32 LinkWithGCC(build_context *BuildContext);