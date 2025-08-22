@echo off

if not %cd% == %workspace_path% (
    echo ERROR: calling a script from outside the workspace root path.
    exit /b 1
)

if "%1"=="" (
    echo ERROR: no argument provided to test.bat
    exit /b 1
)

if "%1"=="build" (
    build build_tests
    %workspace_path%\outputs\build_tests\build_tests.exe
)

if "%1"=="fat12" (
    build fat12_tests
    %workspace_path%\outputs\fat12_tests\fat12_tests.exe
)

if "%1"=="ray_tracer" (
    build ray_tracer 8_lanes && %workspace_path%\outputs\ray_tracer\ray_tracer_8.exe %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp && %workspace_path%\outputs\ray_tracer\test_8_lanes.bmp
    build ray_tracer 4_lanes && %workspace_path%\outputs\ray_tracer\ray_tracer_4.exe %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp && %workspace_path%\outputs\ray_tracer\test_4_lanes.bmp
    build ray_tracer 1_lane && %workspace_path%\outputs\ray_tracer\ray_tracer_1.exe %workspace_path%\outputs\ray_tracer\test_1_lane.bmp && %workspace_path%\outputs\ray_tracer\test_1_lane.bmp
)