@echo off

if not %cd% == %workspace_path% (
    echo ERROR: calling a script from outside the workspace root path.
    exit /b 1
)

if "%1"=="" (
    echo ERROR: no argument provided to run.bat
    exit /b 1
)

if "%1"=="verilog_demo" (
    pushd %workspace_path%\outputs\verilog_demo
        vvp testbench.vvp
        start gtkwave -f demo.vcd
    popd
)

if "%1"=="uart_app" (
    pushd %workspace_path%\outputs\uart_app
        vvp uart_app.vvp
        start gtkwave -f uart_app.vcd
    popd
)

if "%1"=="os" (
    pushd %workspace_path%\outputs\os
        start qemu-system-x86_64 -drive format=raw,file=floppy.img
    popd
)

if "%1"=="build2" (
    outputs\win32\tools\build2\build2.exe %2
)