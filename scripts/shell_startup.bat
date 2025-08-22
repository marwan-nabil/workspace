@echo off

@REM this script is only allowed to run from inside the root workspace folder
@REM TODO: add a check that enforces this
set workspace_path=%cd%

@REM adding cygwin tools to the shell path
set path=%workspace_path%\tools\cygwin\lib\gcc\x86_64-pc-cygwin\12;%path%
set path=%workspace_path%\tools\cygwin\bin;%path%

@REM adding compilers and toolchains to shell path
set path=%workspace_path%\tools\i686-elf\bin;%path%
set path=%workspace_path%\tools\nasm;%path%
set path=%workspace_path%\tools\iverilog\bin;%path%
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

@REM adding simulators and debuggers to shell path
set path=%workspace_path%\tools\qemu;%path%
set path=%workspace_path%\tools\bochs;%path%
set path=%workspace_path%\tools\iverilog\gtkwave\bin;%path%

@REM add local scripts and tools to shell path
set path=%workspace_path%\tools\build;%path%
set path=%workspace_path%\scripts;%path%