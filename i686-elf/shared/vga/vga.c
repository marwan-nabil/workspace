#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\vga\vga.h"

void PrintCharacterColored(char Character, u8 Color, u8 X, u8 Y)
{
    u8 *ScreenBuffer = (u8 *)VGA_SCREEN_BUFFER_ADDRESS;
    u32 ScreenBufferOffset = 2 * (X + Y * VGA_SCREEN_WIDTH);
    ScreenBuffer[ScreenBufferOffset] = Character;
    ScreenBuffer[ScreenBufferOffset + 1] = Color;
}

char GetCharacterAtPosition(u8 X, u8 Y)
{
    u8 *ScreenBuffer = (u8 *)VGA_SCREEN_BUFFER_ADDRESS;
    u32 ScreenBufferOffset = 2 * (X + Y * VGA_SCREEN_WIDTH);
    return (char)ScreenBuffer[ScreenBufferOffset];
}

char GetColorAtPosition(u8 X, u8 Y)
{
    u8 *ScreenBuffer = (u8 *)VGA_SCREEN_BUFFER_ADDRESS;
    u32 ScreenBufferOffset = 2 * (X + Y * VGA_SCREEN_WIDTH);
    return (char)ScreenBuffer[ScreenBufferOffset + 1];
}

void ClearScreen()
{
    for (u8 Y = 0; Y < VGA_SCREEN_HEIGHT; Y++)
    {
        for (u8 X = 0; X < VGA_SCREEN_WIDTH; X++)
        {
            PrintCharacterColored('\0', VGA_DEFAULT_COLOR, X, Y);
        }
    }
}

void ScrollBack(u8 NumberOfLines)
{
    for (u8 Y = NumberOfLines; Y < VGA_SCREEN_HEIGHT; Y++)
    {
        for (u8 X = 0; X < VGA_SCREEN_WIDTH; X++)
        {
            PrintCharacterColored
            (
                GetCharacterAtPosition(X, Y),
                GetColorAtPosition(X, Y),
                X,
                Y - NumberOfLines
            );
        }
    }

    for (u8 Y = VGA_SCREEN_HEIGHT - NumberOfLines; Y < VGA_SCREEN_HEIGHT; Y++)
    {
        for (u8 X = 0; X < VGA_SCREEN_WIDTH; X++)
        {
            PrintCharacterColored('\0', VGA_DEFAULT_COLOR, X, Y);
        }
    }
}