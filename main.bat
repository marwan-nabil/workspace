@echo off
setlocal enabledelayedexpansion

if not exist win32 (
    echo "ERROR: main.bat: not executing from the root of the workspace."
)

if "%1"=="bootstrap" (
    call :bootstrap
    goto :eof
)

if "%1"=="build" (
    build\build\build.exe %*
    goto :eof
)

if "%1"=="compile_only" (
    call toolchains/msvc/setup_x64.bat

    set cc_flags=/nologo /Od /Z7 /Oi /FC /GR- /EHa-
    set cc_flags=!cc_flags! /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
    set cc_flags=!cc_flags! /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
    set cc_flags=!cc_flags! /I%cd%

    if not exist build\compile_only_test; mkdir build\compile_only_test
    pushd build\compile_only_test
        cl /c !cc_flags! ..\..\win32\tests\header_fix_test\main.cpp
    popd
    goto :eof
)

REM if "%1"=="debug" (
REM     if "%1"=="" (
REM         echo ERROR: no argument provided to debug.bat
REM         exit /b 1
REM     )

REM     if "%1"=="build" (
REM         if exist %workspace_path%\configuration\win32\tools\build\build.sln (
REM             start %workspace_path%\configuration\win32\tools\build\build.sln
REM         ) else if exist %workspace_path%\outputs\win32\tools\build\build.exe (
REM             devenv %workspace_path%\outputs\win32\tools\build\build.exe
REM         ) else (
REM             echo ERROR: build.exe doesn't exist.
REM         )
REM     )

REM     if "%1"=="build2" (
REM         if exist %workspace_path%\configuration\win32\tools\build2\build2.sln (
REM             start %workspace_path%\configuration\win32\tools\build2\build2.sln
REM         ) else if exist %workspace_path%\outputs\win32\tools\build2\build2.exe (
REM             devenv %workspace_path%\outputs\win32\tools\build2\build2.exe
REM         ) else (
REM             echo ERROR: build2.exe doesn't exist.
REM         )
REM     )

REM     if "%1"=="bootstrap" (
REM         devenv %workspace_path%\outputs\win32\tools\bootstrapper\bootstrapper.exe
REM     )

REM     if "%1"=="refterm" (
REM         devenv %workspace_path%\outputs\win32\demos\refterm\refterm_debug_msvc.exe
REM     )

REM     if "%1"=="os" (
REM         pushd %workspace_path%\outputs\os
REM             start bochsdbg -q -f %workspace_path%\configuration\i686-elf\os\bochs.txt
REM         popd
REM     )
REM )

REM if "%1"=="run" (
REM     if not %cd% == %workspace_path% (
REM         echo ERROR: calling a script from outside the workspace root path.
REM         exit /b 1
REM     )

REM     if "%1"=="" (
REM         echo ERROR: no argument provided to run.bat
REM         exit /b 1
REM     )

REM     if "%1"=="verilog_demo" (
REM         pushd %workspace_path%\outputs\verilog_demo
REM             vvp testbench.vvp
REM             start gtkwave -f demo.vcd
REM         popd
REM     )

REM     if "%1"=="uart_app" (
REM         pushd %workspace_path%\outputs\uart_app
REM             vvp uart_app.vvp
REM             start gtkwave -f uart_app.vcd
REM         popd
REM     )

REM     if "%1"=="os" (
REM         pushd %workspace_path%\outputs\os
REM             start qemu-system-x86_64 -drive format=raw,file=floppy.img
REM         popd
REM     )

REM     if "%1"=="build2" (
REM         outputs\win32\tools\build2\build2.exe %2
REM     )
REM )

REM if "%1"=="test" (
REM     if not %cd% == %workspace_path% (
REM         echo ERROR: calling a script from outside the workspace root path.
REM         exit /b 1
REM     )

REM     if "%1"=="" (
REM         echo ERROR: no argument provided to test.bat
REM         exit /b 1
REM     )

REM     if "%1"=="build" (
REM         build build_tests
REM         %workspace_path%\outputs\build_tests\build_tests.exe
REM     )

REM     if "%1"=="fat12" (
REM         build fat12_tests
REM         %workspace_path%\outputs\fat12_tests\fat12_tests.exe
REM     )

REM     if "%1"=="ray_tracer" (
REM         build ray_tracer 8_lanes && %workspace_path%\outputs\ray_tracer\ray_tracer_8.exe %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp && %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp
REM         build ray_tracer 4_lanes && %workspace_path%\outputs\ray_tracer\ray_tracer_4.exe %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp && %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp
REM         build ray_tracer 1_lane && %workspace_path%\outputs\ray_tracer\ray_tracer_1.exe %workspace_path%\outputs\ray_tracer\test_1_lane.bmp && %workspace_path%\outputs\ray_tracer\test_1_lane.bmp
REM     )
REM )

:bootstrap
    call toolchains\msvc\setup_x64.bat
    set cc_flags=/nologo /Od /Z7 /Oi /FC /GR- /EHa-
    set cc_flags=!cc_flags! /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
    set cc_flags=!cc_flags! /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
    set cc_flags=!cc_flags! /I%cd%
    set link_flags=/subsystem:console /incremental:no /opt:ref user32.lib shell32.lib Shlwapi.lib

    if not exist build\build; mkdir build\build
    pushd build\build
        cl !cc_flags!^
            ..\..\win32\build\build.cpp^
            ..\..\win32\shared\shell\console.cpp^
            ..\..\win32\shared\file_system\folders.cpp^
            ..\..\win32\shared\strings\string_list.cpp^
            /Fe:build.exe^
            /link !link_flags!
    popd
goto :eof