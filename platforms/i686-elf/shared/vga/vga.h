#pragma once

#define VGA_SCREEN_BUFFER_ADDRESS 0xB8000
#define VGA_SCREEN_WIDTH 80
#define VGA_SCREEN_HEIGHT 25
#define VGA_DEFAULT_COLOR 0x07

void PrintCharacterColored(char Character, u8 Color, u8 X, u8 Y);
char GetCharacterAtPosition(u8 X, u8 Y);
char GetColorAtPosition(u8 X, u8 Y);
void ClearScreen();
void ScrollBack(u8 NumberOfLines);