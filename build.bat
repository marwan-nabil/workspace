@echo off
setlocal enabledelayedexpansion

if not exist win32 (
    echo "ERROR: build.bat: not executing from the root of the workspace."
    goto :eof
)

if "%1"=="clean" (
    if exist build; rmdir /Q /S build
    goto :eof
) else if "%1"=="bootstrapper" (
    call :build_bootstrapper
    goto :eof
) else if "%1"=="compile_only" (
    call toolchains/msvc/setup_x64.bat
    set cc_flags=/nologo /Od /Z7 /Oi /FC /GR- /EHa-
    set cc_flags=!cc_flags! /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
    set cc_flags=!cc_flags! /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
    set cc_flags=!cc_flags! /I%cd%
    if not exist build\compile_only; mkdir build\compile_only
    pushd build\compile_only
    cl /c !cc_flags! ..\..\win32\tests\header_fix_test\main.cpp
    popd
    goto :eof
) else if "%1"=="cstrings_test" (
    call toolchains\msvc\setup_x64.bat
    call toolchains\nasm\setup.bat
    if not exist build\cstrings_test; mkdir build\cstrings_test
    pushd build\cstrings_test
    nasm ^
        -g -f win64 -o cstrings.obj -I%cd%^
        %cd%\win32\shared\strings\cstrings.asm
    nasm ^
        -g -f win64 -o linear_allocator.obj -I%cd%^
        %cd%\win32\shared\memory\linear_allocator.asm
    nasm ^
        -g -f win64 -o singly_linked_list.obj -I%cd%^
        %cd%\win32\shared\structures\singly_linked_list.asm
    nasm ^
        -g -f win64 -o cstrings_test.obj -I%cd%^
        %cd%\win32\tests\assembly_tests\cstrings_test.asm
    link ^
        /debug /map /subsystem:console /out:cstrings_test.exe^
        cstrings_test.obj cstrings.obj linear_allocator.obj singly_linked_list.obj ^
        kernel32.lib legacy_stdio_definitions.lib msvcrt.lib
    popd
    goto :eof
) else if "%1"=="memory_test" (
    call toolchains\msvc\setup_x64.bat
    call toolchains\nasm\setup.bat
    if not exist build\memory_test; mkdir build\memory_test
    pushd build\memory_test
    nasm ^
        -g -f win64 -o linear_allocator.obj -I%cd%^
        %cd%\win32\shared\memory\linear_allocator.asm
    nasm ^
        -g -f win64 -o memory_test.obj -I%cd%^
        %cd%\win32\tests\assembly_tests\memory_test.asm
    link ^
        /debug /map /subsystem:console /out:memory_test.exe^
        memory_test.obj linear_allocator.obj kernel32.lib legacy_stdio_definitions.lib msvcrt.lib
    popd
    goto :eof
) else (
    call toolchains\msvc\setup_x64.bat
    if not exist build\bootstrapper\bootstrapper.exe; call :build_bootstrapper
    build\bootstrapper\bootstrapper.exe %*
    build\build\build.exe %*
    goto :eof
)

@REM if "%1"=="test" (
@REM     if "%1"=="build" (
@REM         build build_tests
@REM         %workspace_path%\outputs\build_tests\build_tests.exe
@REM     )
@REM     if "%1"=="fat12" (
@REM         build fat12_tests
@REM         %workspace_path%\outputs\fat12_tests\fat12_tests.exe
@REM     )
@REM     if "%1"=="ray_tracer" (
@REM         build ray_tracer 8_lanes &&^
@REM             %workspace_path%\outputs\ray_tracer\ray_tracer_8.exe %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp &&^
@REM             %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp
@REM         build ray_tracer 4_lanes &&^
@REM             %workspace_path%\outputs\ray_tracer\ray_tracer_4.exe %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp &&^
@REM             %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp
@REM         build ray_tracer 1_lane &&^
@REM             %workspace_path%\outputs\ray_tracer\ray_tracer_1.exe %workspace_path%\outputs\ray_tracer\test_1_lane.bmp &&^
@REM             %workspace_path%\outputs\ray_tracer\test_1_lane.bmp
@REM     )
@REM )

:build_bootstrapper
    echo INFO: build.bat: building bootstrapper started.
    call toolchains\msvc\setup_x64.bat
    set cc_flags=/nologo /Od /Z7 /Oi /FC /GR- /EHa-
    set cc_flags=!cc_flags! /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
    set cc_flags=!cc_flags! /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
    set cc_flags=!cc_flags! /I%cd%
    set link_flags=/subsystem:console /incremental:no /opt:ref user32.lib shell32.lib Shlwapi.lib
    if not exist build\bootstrapper; mkdir build\bootstrapper
    pushd build\bootstrapper
    cl !cc_flags!^
        ..\..\win32\applications\bootstrapper\bootstrapper.cpp^
        ..\..\win32\shared\shell\console.cpp^
        ..\..\win32\shared\file_system\folders.cpp^
        ..\..\win32\shared\file_system\files.cpp^
        ..\..\win32\shared\system\processes.cpp^
        /Fe:bootstrapper.exe^
        /link !link_flags!
    popd
    echo INFO: build.bat: building bootstrapper succeeded.
goto :eof