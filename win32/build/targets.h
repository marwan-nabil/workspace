#pragma once

#include "win32\shared\base_types.h"
#include "win32\build\build.h"

#define NUMBER_OF_CONFIGURED_TARGETS 1

typedef b8 (target_entry_function_type)(environment_info *);

struct target_configuration
{
    char *TargetName;
    target_entry_function_type *TargetEntryFunction;
};

extern target_configuration GlobalTargetConfigurations[NUMBER_OF_CONFIGURED_TARGETS];

b8 BuildLintTarget(environment_info *EnvironmentInfo);