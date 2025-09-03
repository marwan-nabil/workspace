#pragma once

b32 CreateProcessAndWait(char *CommandLine, HANDLE ProcessOutput, console_context *ConsoleContext);
b32 CreateProcessAndWait(char *CommandLine, console_context *ConsoleContext);