@echo off
setlocal

call toolchains/msvc/setup_x64.bat

if "%1"=="debug" (
    set optimization_flags=/Od /Z7
) else (
    set optimization_flags=/O2
)
set cc_flags=/nologo %optimization_flags% /Oi /FC /GR- /EHa-
set cc_flags=%cc_flags% /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
set cc_flags=%cc_flags% /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS
set cc_flags=%cc_flags% /I%cd%
set link_flags=/link /subsystem:console /incremental:no /opt:ref user32.lib shell32.lib Shlwapi.lib

if not exist %cd%\build\bootstrapper; mkdir %cd%\build\bootstrapper
pushd %cd%\build\bootstrapper
    cl %cc_flags%^
        %cd%\..\..\win32\bootstrapper\bootstrapper.cpp^
        %cd%\..\..\win32\build\actions\build_context.cpp^
        %cd%\..\..\win32\build\actions\msvc.cpp^
        %cd%\..\..\win32\shared\file_system\folders.cpp^
        %cd%\..\..\win32\shared\shell\console.cpp^
        %cd%\..\..\win32\shared\strings\path_handling.cpp^
        %cd%\..\..\win32\shared\strings\string_list.cpp^
        %cd%\..\..\win32\shared\system\processes.cpp^
        /Fe:bootstrapper.exe^
        %link_flags%
popd

%cd%\build\bootstrapper\bootstrapper.exe %1

if not exist %cd%\build\build\build.exe (
    echo ERROR: bootstrap.bat: build.exe was not bootstrapped successfully.
)