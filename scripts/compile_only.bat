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

if not exist %cd%\build\compile_only_test; mkdir %cd%\build\compile_only_test
pushd %cd%\build\compile_only_test
    cl /c %cc_flags%^
        %cd%\..\..\win32\bootstrapper\bootstrapper.cpp
popd
