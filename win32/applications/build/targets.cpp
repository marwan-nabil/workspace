#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#include <strsafe.h>
#include <io.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

#include "win32\applications\build\targets.h"

target_configuration GlobalTargetConfigurations[NUMBER_OF_CONFIGURED_TARGETS] =
{
    {"lint", &BuildLintTarget},
};