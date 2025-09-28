#include <stdint.h>
#include <math.h>
#include <intrin.h>
#include <string.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\math\constants.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\bit_operations.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\transcendentals.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\math\rectangle2.h"
#include "win32\shared\math\rectangle3.h"

#include "game_interface.h"
#include "memory.h"
#include "bitmap.h"
#include "renderer.h"
#include "random_numbers_table.h"

#include "entity.h"
#include "collision.h"
#include "world.h"
#include "simulation.h"
#include "game.h"

loaded_bitmap LoadBitmap(platform_read_file *ReadEntireFile, thread_context *ThreadContext, char *FileName)
{
    loaded_bitmap Result = {};

    read_file_result ReadResult = ReadEntireFile(ThreadContext, FileName);
    if (ReadResult.ConstentsSize)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.FileContents;
        Assert(Header->Compression == 3);

        u32 SourceRedMask = Header->RedMask;
        u32 SourceGreenMask = Header->GreenMask;
        u32 SourceBlueMask = Header->BlueMask;
        u32 SourceAlphaMask = ~(SourceRedMask | SourceGreenMask | SourceBlueMask);

        bitscan_result BitscanResult = FindLeastSignificantSetBit(SourceRedMask);
        Assert(BitscanResult.Found);
        u32 SourceRedShift = BitscanResult.Index;

        BitscanResult = FindLeastSignificantSetBit(SourceGreenMask);
        Assert(BitscanResult.Found);
        u32 SourceGreenShift = BitscanResult.Index;

        BitscanResult = FindLeastSignificantSetBit(SourceBlueMask);
        Assert(BitscanResult.Found);
        u32 SourceBlueShift = BitscanResult.Index;

        BitscanResult = FindLeastSignificantSetBit(SourceAlphaMask);
        Assert(BitscanResult.Found);
        u32 SourceAlphaShift = BitscanResult.Index;

        // calculate needed bit rotations to the left
        i32 AlphaTargetLeftRotation = 24 - (i32)SourceAlphaShift;
        i32 RedTargetLeftRotation = 16 - (i32)SourceRedShift;
        i32 GreenTargetLeftRotation = 8 - (i32)SourceGreenShift;
        i32 BlueTargetLeftRotation = 0 - (i32)SourceBlueShift;

        Result.Pixels = (u32 *)((u8 *)ReadResult.FileContents + Header->BitmapOffset);

        // NOTE: target pixel format 0x AA RR GG BB
        u32 *SourceAndDest = Result.Pixels;
        for (i32 Y = 0; Y < Header->Height; Y++)
        {
            for (i32 X = 0; X < Header->Width; X++)
            {
                u32 Value = *SourceAndDest;
                *SourceAndDest++ =
                    RotateLeft(Value & SourceAlphaMask, AlphaTargetLeftRotation) |
                    RotateLeft(Value & SourceRedMask, RedTargetLeftRotation) |
                    RotateLeft(Value & SourceGreenMask, GreenTargetLeftRotation) |
                    RotateLeft(Value & SourceBlueMask, BlueTargetLeftRotation);
            }
        }

        Result.Width = Header->Width;
        Result.Height = Header->Height;
    }
    return Result;
}