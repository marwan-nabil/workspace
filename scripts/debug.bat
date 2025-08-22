@echo off

if not %cd% == %workspace_path% (
    echo ERROR: calling a script from outside the workspace root path.
    exit /b 1
)

if "%1"=="" (
    echo ERROR: no argument provided to debug.bat
    exit /b 1
)

if "%1"=="build" (
    if exist %workspace_path%\configuration\win32\tools\build\build.sln (
        start %workspace_path%\configuration\win32\tools\build\build.sln
    ) else if exist %workspace_path%\outputs\win32\tools\build\build.exe (
        devenv %workspace_path%\outputs\win32\tools\build\build.exe
    ) else (
        echo ERROR: build.exe doesn't exist.
    )
)

if "%1"=="build2" (
    if exist %workspace_path%\configuration\win32\tools\build2\build2.sln (
        start %workspace_path%\configuration\win32\tools\build2\build2.sln
    ) else if exist %workspace_path%\outputs\win32\tools\build2\build2.exe (
        devenv %workspace_path%\outputs\win32\tools\build2\build2.exe
    ) else (
        echo ERROR: build2.exe doesn't exist.
    )
)

if "%1"=="bootstrap" (
    devenv %workspace_path%\outputs\win32\tools\bootstrapper\bootstrapper.exe
)

if "%1"=="refterm" (
    devenv %workspace_path%\outputs\win32\demos\refterm\refterm_debug_msvc.exe
)

if "%1"=="os" (
    pushd %workspace_path%\outputs\os
        start bochsdbg -q -f %workspace_path%\configuration\i686-elf\os\bochs.txt
    popd
)