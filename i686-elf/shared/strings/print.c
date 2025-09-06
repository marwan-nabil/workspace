#include <stdarg.h>
#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\memory\memory.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\vga\vga.h"
#include "i686-elf\shared\cpu\io.h"

void SetCursorPosition(print_context *Context, u32 X, u32 Y)
{
    u32 LinearPosition = X + Y * VGA_SCREEN_WIDTH;
    WriteByteToOutputPort(0x03D4, 0x0F);
    WriteByteToOutputPort(0x03D5, (u8)(LinearPosition & 0xFF));
    WriteByteToOutputPort(0x03D4, 0x0E);
    WriteByteToOutputPort(0x03D5, (u8)((LinearPosition >> 8) & 0xFF));
    Context->X = X;
    Context->Y = Y;
}

void PrintCharacter(print_context *Context, char Character)
{
    WriteByteToOutputPort(0xE9, Character);

    switch (Character)
    {
        case '\n':
        {
            Context->X = 0;
            Context->Y++;
        } break;

        case '\t':
        {
            for (u8 Index = 0; Index < 4 - (Context->X % 4); Index++)
            {
                PrintCharacter(Context, ' ');
            }
        } break;

        case '\r':
        {
            Context->X = 0;
        } break;

        default:
        {
            PrintCharacterColored(Character, VGA_DEFAULT_COLOR, Context->X, Context->Y);
            Context->X++;
        } break;
    }

    if (Context->X >= VGA_SCREEN_WIDTH)
    {
        Context->Y++;
        Context->X = 0;
    }

    if (Context->Y >= VGA_SCREEN_HEIGHT)
    {
        ScrollBack(1);
        Context->Y -= 1;
    }

    SetCursorPosition(Context, Context->X, Context->Y);
}

void PrintString(print_context *Context, char *String)
{
    while (*String)
    {
        PrintCharacter(Context, *String);
        String++;
    }
}

void PrintFormattedNumberUnsigned(u64 Number, u32 Radix, print_context *Context)
{
    const char HexCharacters[] = "0123456789abcdef";

    char LocalStringBuffer[32];
    MemoryZero((void *)LocalStringBuffer, 32);

    i32 StringBufferPosition = 0;

    do
    {
        u64 Remainder = Number % Radix;
        Number /= Radix;

        LocalStringBuffer[StringBufferPosition++] = HexCharacters[Remainder];
    } while (Number > 0);

    while (--StringBufferPosition >= 0)
    {
        PrintCharacter(Context, LocalStringBuffer[StringBufferPosition]);
    }
}

void PrintFormattedNumberSigned(u64 Number, u32 Radix, print_context *Context)
{
    if (Number < 0)
    {
        PrintCharacter(Context, '-');
        PrintFormattedNumberUnsigned(-Number, Radix, Context);
    }
    else
    {
        PrintFormattedNumberUnsigned(Number, Radix, Context);
    }
}

void PrintFormatted(print_context *Context, const char *FormatString, ...)
{
    // TODO: fix negative numbers printing bug in PrintFormatted
    va_list ArgumentsList;
    va_start(ArgumentsList, FormatString);

    printf_state FormatStringState = PRINTF_STATE_NORMAL;
    printf_length_type LengthType = PRINTF_LENGTH_TYPE_DEFAULT;
    u32 Radix = 10;
    b8 IsSigned = FALSE;
    b8 IsNumber = FALSE;

    while (*FormatString)
    {
        switch (FormatStringState)
        {
            case PRINTF_STATE_NORMAL:
            {
                if (*FormatString == '%')
                {
                    FormatStringState = PRINTF_STATE_CHECK_LENGTH;
                }
                else
                {
                    PrintCharacter(Context, *FormatString);
                }
                FormatString++;
            } break;

            case PRINTF_STATE_CHECK_LENGTH:
            {
                if (*FormatString == 'h')
                {
                    LengthType = PRINTF_LENGTH_TYPE_SHORT;
                    FormatStringState = PRINTF_STATE_SHORT_LENGTH;
                    FormatString++;
                }
                else if (*FormatString == 'l')
                {
                    LengthType = PRINTF_LENGTH_TYPE_LONG;
                    FormatStringState = PRINTF_STATE_LONG_LENGTH;
                    FormatString++;
                }
                else
                {
                    FormatStringState = PRINTF_STATE_CHECK_SPECIFIER;
                }
            } break;

            case PRINTF_STATE_SHORT_LENGTH:
            {
                if (*FormatString == 'h')
                {
                    LengthType = PRINTF_LENGTH_TYPE_SHORT_SHORT;
                    FormatStringState = PRINTF_STATE_CHECK_SPECIFIER;
                    FormatString++;
                }
                else
                {
                    FormatStringState = PRINTF_STATE_CHECK_SPECIFIER;
                }
            } break;

            case PRINTF_STATE_LONG_LENGTH:
            {
                if (*FormatString == 'l')
                {
                    LengthType = PRINTF_LENGTH_TYPE_LONG_LONG;
                    FormatStringState = PRINTF_STATE_CHECK_SPECIFIER;
                    FormatString++;
                }
                else
                {
                    FormatStringState = PRINTF_STATE_CHECK_SPECIFIER;
                }
            } break;

            case PRINTF_STATE_CHECK_SPECIFIER:
            {
                switch (*FormatString)
                {
                    case 'c':
                    {
                        PrintCharacter(Context, (char)va_arg(ArgumentsList, i32));
                    } break;

                    case 's':
                    {
                        PrintString(Context, va_arg(ArgumentsList, char *));
                    } break;

                    case '%':
                    {
                        PrintCharacter(Context, '%');
                    } break;

                    case 'd':
                    case 'i':
                    {
                        Radix = 10;
                        IsSigned = TRUE;
                        IsNumber = TRUE;
                    } break;

                    case 'u':
                    {
                        Radix = 10;
                        IsSigned = FALSE;
                        IsNumber = TRUE;
                    } break;

                    case 'X':
                    case 'x':
                    case 'p':
                    {
                        Radix = 16;
                        IsSigned = FALSE;
                        IsNumber = TRUE;
                    } break;

                    case 'o':
                    {
                        Radix = 8;
                        IsSigned = FALSE;
                        IsNumber = TRUE;
                    } break;

                    default:
                    {
                    } break;
                }

                if (IsNumber)
                {
                    if (IsSigned)
                    {
                        switch (LengthType)
                        {
                            case PRINTF_LENGTH_TYPE_SHORT_SHORT:
                            case PRINTF_LENGTH_TYPE_SHORT:
                            case PRINTF_LENGTH_TYPE_DEFAULT:
                            {
                                PrintFormattedNumberSigned(va_arg(ArgumentsList, i32), Radix, Context);
                            } break;

                            case PRINTF_LENGTH_TYPE_LONG:
                            case PRINTF_LENGTH_TYPE_LONG_LONG:
                            {
                                PrintFormattedNumberSigned(va_arg(ArgumentsList, i64), Radix, Context);
                            } break;
                        }
                    }
                    else
                    {
                        switch (LengthType)
                        {
                            case PRINTF_LENGTH_TYPE_SHORT_SHORT:
                            case PRINTF_LENGTH_TYPE_SHORT:
                            case PRINTF_LENGTH_TYPE_DEFAULT:
                            {
                                PrintFormattedNumberUnsigned(va_arg(ArgumentsList, u32), Radix, Context);
                            } break;

                            case PRINTF_LENGTH_TYPE_LONG:
                            case PRINTF_LENGTH_TYPE_LONG_LONG:
                            {
                                PrintFormattedNumberUnsigned(va_arg(ArgumentsList, u64), Radix, Context);
                            } break;
                        }
                    }
                }

                FormatStringState = PRINTF_STATE_NORMAL;
                LengthType = PRINTF_LENGTH_TYPE_DEFAULT;
                Radix = 10;
                IsSigned = FALSE;
                IsNumber = FALSE;

                FormatString++;
            } break;
        }
    }
}