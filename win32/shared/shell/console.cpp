#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\shell\console.h"

void InitializeConsole(console_context *ConsoleContext)
{
    ConsoleContext->ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
    GetConsoleScreenBufferInfo(ConsoleContext->ConsoleHandle, &ConsoleInfo);
    ConsoleContext->OriginalConsoleAttributes = ConsoleInfo.wAttributes;
}

void ConsoleSwitchColor(WORD Color, console_context *ConsoleContext)
{
    SetConsoleTextAttribute(ConsoleContext->ConsoleHandle, Color);
}

void ConsoleResetColor(console_context *ConsoleContext)
{
    SetConsoleTextAttribute(ConsoleContext->ConsoleHandle, ConsoleContext->OriginalConsoleAttributes);
}

void ConsolePrintColored(const char *String, WORD Color, console_context *ConsoleContext)
{
    SetConsoleTextAttribute(ConsoleContext->ConsoleHandle, Color);
    printf(String);
    fflush(stdout);
    SetConsoleTextAttribute(ConsoleContext->ConsoleHandle, ConsoleContext->OriginalConsoleAttributes);
}