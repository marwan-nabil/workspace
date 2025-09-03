#pragma once

typedef enum
{
    PRINTF_STATE_NORMAL,
    PRINTF_STATE_CHECK_LENGTH,

    PRINTF_STATE_SHORT_LENGTH,
    PRINTF_STATE_LONG_LENGTH,

    PRINTF_STATE_CHECK_SPECIFIER
} printf_state;

typedef enum
{
    PRINTF_LENGTH_TYPE_SHORT_SHORT,
    PRINTF_LENGTH_TYPE_SHORT,
    PRINTF_LENGTH_TYPE_DEFAULT,
    PRINTF_LENGTH_TYPE_LONG,
    PRINTF_LENGTH_TYPE_LONG_LONG
} printf_length_type;

typedef struct
{
    u32 X;
    u32 Y;
} print_context;

void SetCursorPosition(print_context *Context, u32 X, u32 Y);
void PrintCharacter(print_context *Context, char Character);
void PrintString(print_context *Context, char *String);
void PrintFormattedNumberUnsigned(u64 Number, u32 Radix, print_context *Context);
void PrintFormattedNumberSigned(u64 Number, u32 Radix, print_context *Context);
void PrintFormatted(print_context *Context, const char *FormatString, ...);