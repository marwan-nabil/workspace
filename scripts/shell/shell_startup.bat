@echo off

set workspace_path=%cd%
set path=%workspace_path%\scripts;%path%
set path=%workspace_path%\tools\bazel;%path%
set BAZEL_SH=%workspace_path%\toolchains\mysys64\usr\bin\bash.exe