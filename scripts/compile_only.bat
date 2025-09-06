@echo off

if not %cd% == %workspace_path% (
    echo ERROR: calling a script from outside the workspace root path.
    exit /b 1
)

set cc_flags_0=/nologo /FC /GR- /EHa- /O2 /I%workspace_path%
set cc_flags_1=/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018
set cc_flags_2=/D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS

if not exist %workspace_path%\outputs\compile_only_test; mkdir %workspace_path%\outputs\compile_only_test
pushd %workspace_path%\outputs\compile_only_test
    cl /c^
        %cc_flags_0%^
        %cc_flags_1%^
        %cc_flags_2%^
        %workspace_path%\win32\tools\build2\test.cpp ^
        %workspace_path%\win32\tools\build2\memory.cpp ^
        %workspace_path%\win32\tools\build2\entity.cpp ^
        %workspace_path%\win32\tools\build2\build2.cpp
popd