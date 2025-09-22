#pragma once

#include <Windows.h>

struct console_context
{
    HANDLE ConsoleHandle;
    WORD OriginalConsoleAttributes;
};

void InitializeConsole(console_context *ConsoleContext);
void ConsoleSwitchColor(WORD Color, console_context *ConsoleContext);
void ConsoleResetColor(console_context *ConsoleContext);
void ConsolePrintColored(const char *String, WORD Color, console_context *ConsoleContext);