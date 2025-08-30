@echo off

if not %cd% == %workspace_path% (
    echo ERROR: calling a script from outside the workspace root path.
    exit /b 1
)

@REM set compilation and linking flags
if "%1"=="debug" (
    set optimization_flags=/Od /Z7
) else (
    set optimization_flags=/O2
)
set cc_flags=/nologo %optimization_flags% /Oi /FC /GR- /EHa-
set cc_flags=%cc_flags% /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
set cc_flags=%cc_flags% /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
set cc_flags=%cc_flags% /I%workspace_path%
set link_flags=/link /subsystem:console /incremental:no /opt:ref user32.lib shell32.lib Shlwapi.lib

@REM TODO: make the outputs/ folder path configurable
if not exist %workspace_path%\outputs\win32\tools\bootstrapper; mkdir %workspace_path%\outputs\win32\tools\bootstrapper
pushd %workspace_path%\outputs\win32\tools\bootstrapper
    cl %cc_flags%^
        %workspace_path%\sources\win32\tools\bootstrapper\bootstrapper.cpp^
        %workspace_path%\sources\win32\tools\build\actions\build_context.cpp^
        %workspace_path%\sources\win32\tools\build\actions\msvc.cpp^
        %workspace_path%\sources\win32\libraries\file_system\folders.cpp^
        %workspace_path%\sources\win32\libraries\shell\console.cpp^
        %workspace_path%\sources\win32\libraries\strings\path_handling.cpp^
        %workspace_path%\sources\win32\libraries\strings\string_list.cpp^
        %workspace_path%\sources\win32\libraries\system\processes.cpp^
        /Fe:bootstrapper.exe^
        %link_flags%
popd

%workspace_path%\outputs\win32\tools\bootstrapper\bootstrapper.exe %1

if exist %workspace_path%\outputs\win32\tools\build\build.exe (
    if not exist %workspace_path%\tools\build; mkdir %workspace_path%\tools\build
    copy %workspace_path%\outputs\win32\tools\build\build.exe %workspace_path%\tools\build\build.exe
) else (
    echo ERROR: build.exe was not built!
)