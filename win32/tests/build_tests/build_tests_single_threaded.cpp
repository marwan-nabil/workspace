#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <strsafe.h>
#include <io.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\system\processes.h"

void CompilationTest(char *TestCommand)
{
    printf("\n");
    ConsoleSwitchColor(BACKGROUND_BLUE);
    printf("> %s", TestCommand);
    ConsoleResetColor();
    printf("\n");
    fflush(stdout);

    b32 Result = CreateProcessAndWait(TestCommand);
    if (Result)
    {
        ConsolePrintColored("INFO: test succeeded.\n", FOREGROUND_GREEN);
    }
    else
    {
        ConsolePrintColored("ERROR: test failed.\n", FOREGROUND_RED);
        exit(1);
    }
}

i32 main(i32 argc, char **argv)
{
    InitializeConsole();

    CompilationTest("build simulator");
    CompilationTest("build imgui_demo opengl2");
    CompilationTest("build imgui_demo dx11");
    CompilationTest("build ray_tracer 1_lane");
    CompilationTest("build ray_tracer 4_lanes");
    CompilationTest("build ray_tracer 8_lanes");
    CompilationTest("build handmade_hero");
    CompilationTest("build directx_demo debug");
    CompilationTest("build directx_demo release");
    CompilationTest("build lint");
    CompilationTest("build fetch_data");
    CompilationTest("build fat12_tests");
    CompilationTest("build build_tests_multi_threaded");
    CompilationTest("build os");
    CompilationTest("build x86_kernel_tests");

    ConsolePrintColored
    (
        "\n==========================\n"
        "INFO: all tests succeeded.\n"
        "==========================\n",
        FOREGROUND_GREEN
    );
}