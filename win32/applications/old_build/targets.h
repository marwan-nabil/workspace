#pragma once

#include "portable\shared\base_types.h"
#include "win32\build\actions\build_context.h"

#define CONFIGURED_TARGETS_COUNT 13

typedef b32 (build_function_type)(build_context *);

struct build_target_config
{
    const char *TargetName;
    build_function_type *BuildFunction;
    const char *FirstArgument;
    const char *SecondArgument;
    const char *ThirdArgument;
};

extern build_target_config BuildTargetConfigurations[CONFIGURED_TARGETS_COUNT];