@echo off
setlocal enabledelayedexpansion

if "%1"=="memory_test" (
    build\memory_test\memory_test.exe
) else if "%1"=="cstrings_test" (
    build\cstrings_test\cstrings_test.exe
) else if "%1"=="lint" (
    build\lint\lint.exe
) else if "%1"=="verilog_demo" (
    pushd build\verilog_demo
    vvp testbench.vvp
    start gtkwave -f demo.vcd
    popd
) else if "%1"=="uart_app" (
    pushd build\uart_app
    vvp uart_app.vvp
    start gtkwave -f uart_app.vcd
    popd
) else if "%1"=="os" (
    pushd build\os
    start qemu-system-x86_64 -drive format=raw,file=floppy.img
    popd
)