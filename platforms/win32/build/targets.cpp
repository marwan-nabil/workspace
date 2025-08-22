#include <Windows.h>
#include <stdint.h>

#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\strings\string_list.h"
#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\targets.h"
#include "sources\win32\tools\build\build.h"
#include "sources\win32\tools\build\targets\hdl\uart_app\build.h"
#include "sources\win32\tools\build\targets\hdl\verilog_demo\build.h"
#include "sources\win32\tools\build\targets\i686-elf\os\build.h"
#include "sources\win32\tools\build\targets\win32\lint\build.h"
#include "sources\win32\tools\build\targets\win32\fetch_data\build.h"
#include "sources\win32\tools\build\targets\win32\build_tests\build.h"
#include "sources\win32\tools\build\targets\win32\fat12_tests\build.h"
#include "sources\win32\tools\build\targets\win32\simulator\build.h"
#include "sources\win32\tools\build\targets\win32\directx_demo\build.h"
#include "sources\win32\tools\build\targets\win32\handmade_hero\build.h"
#include "sources\win32\tools\build\targets\win32\imgui_demo\build.h"
#include "sources\win32\tools\build\targets\win32\ray_tracer\build.h"
#include "sources\win32\tools\build\targets\win32\refterm\build.h"

build_target_config BuildTargetConfigurations[CONFIGURED_TARGETS_COUNT] =
{
    {"lint", &BuildLint, "[job_per_directory]", NULL, NULL},
    {"fetch_data", &BuildFetchData, NULL, NULL, NULL},
    {"build_tests", &BuildBuildTests, NULL, NULL, NULL},
    {"fat12_tests", &BuildFat12Tests, NULL, NULL, NULL},
    {"simulator", &BuildSimulator, NULL, NULL, NULL},
    {"directx_demo", &BuildDirectxDemo, NULL, NULL, NULL},
    {"handmade_hero", &BuildHandmadeHero, NULL, NULL, NULL},
    {"imgui_demo", &BuildImguiDemo, "[opengl2, dx11]", NULL, NULL},
    {"ray_tracer", &BuildRayTracer, "[1_lane, 4_lanes, 8_lanes]", NULL, NULL},
    {"os", &BuildOsFloppyDiskImage, NULL, NULL, NULL},
    {"verilog_demo", &BuildVerilogDemo, NULL, NULL, NULL},
    {"uart_app", &BuildUARTApp, NULL, NULL, NULL},
    {"refterm", &BuildRefTerm, NULL, NULL, NULL},
};