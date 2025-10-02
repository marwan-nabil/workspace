#include <stdint.h>
#include <math.h>
#include <intrin.h>
#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
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

void DrawRectangle
(
    game_pixel_buffer *PixelBuffer,
    v2 MinCorner, v2 MaxCorner,
    f32 Red, f32 Green, f32 Blue, f32 Alpha
)
{
    i32 MinX = RoundF32ToI32(MinCorner.X);
    i32 MinY = RoundF32ToI32(MinCorner.Y);
    i32 MaxX = RoundF32ToI32(MaxCorner.X);
    i32 MaxY = RoundF32ToI32(MaxCorner.Y);

    if (MinX < 0)
    {
        MinX = 0;
    }
    if (MinY < 0)
    {
        MinY = 0;
    }
    if (MaxX > PixelBuffer->WidthInPixels)
    {
        MaxX = PixelBuffer->WidthInPixels;
    }
    if (MaxY > PixelBuffer->HeightInPixels)
    {
        MaxY = PixelBuffer->HeightInPixels;
    }

    u32 Color = (u32)
    (
        RoundF32ToU32(Alpha * 255.0f) << 24 |
        RoundF32ToU32(Red * 255.0f) << 16 |
        RoundF32ToU32(Green * 255.0f) << 8 |
        RoundF32ToU32(Blue * 255.0f)
    );

    u8 *Row = ((u8 *)PixelBuffer->PixelsMemory + MinX * PixelBuffer->BytesPerPixel + MinY * PixelBuffer->BytesPerRow);

    for (i32 Y = MinY; Y < MaxY; Y++)
    {
        u32 *Pixel = (u32 *)Row;

        for (i32 X = MinX; X < MaxX; X++)
        {
            f32 SourceAlpha = (f32)((Color >> 24) & 0xff) / 255.0f;
            f32 SourceRed = (f32)((Color >> 16) & 0xff);
            f32 SourceGreen = (f32)((Color >> 8) & 0xff);
            f32 SourceBlue = (f32)((Color >> 0) & 0xff);

            f32 DestinationRed = (f32)((*Pixel >> 16) & 0xff);
            f32 DestinationGreen = (f32)((*Pixel >> 8) & 0xff);
            f32 DestinationBlue = (f32)((*Pixel >> 0) & 0xff);

            f32 ResultRed = SourceAlpha * SourceRed + (1 - SourceAlpha) * DestinationRed;
            f32 ResultGreen = SourceAlpha * SourceGreen + (1 - SourceAlpha) * DestinationGreen;
            f32 ResultBlue = SourceAlpha * SourceBlue + (1 - SourceAlpha) * DestinationBlue;

            *Pixel++ =
                ((u32)(ResultRed + 0.5f) << 16) |
                ((u32)(ResultGreen + 0.5f) << 8) |
                ((u32)(ResultBlue + 0.5f) << 0);
        }

        Row += PixelBuffer->BytesPerRow;
    }
}

void DrawBitmap
(
    loaded_bitmap *SourceBitMap, game_pixel_buffer *DestinationBuffer,
    f32 DestinationX, f32 DestinationY, f32 AlphaFactor
)
{
    i32 DestMinX = RoundF32ToI32(DestinationX);
    i32 DestMinY = RoundF32ToI32(DestinationY);
    i32 DestMaxX = DestMinX + SourceBitMap->Width;
    i32 DestMaxY = DestMinY + SourceBitMap->Height;

    i32 SourceOffsetX = 0;
    i32 SourceOffsetY = 0;

    if (DestMinX < 0)
    {
        SourceOffsetX = -DestMinX;
        DestMinX = 0;
    }
    if (DestMinY < 0)
    {
        SourceOffsetY = -DestMinY;
        DestMinY = 0;
    }
    if (DestMaxX > DestinationBuffer->WidthInPixels)
    {
        DestMaxX = DestinationBuffer->WidthInPixels;
    }
    if (DestMaxY > DestinationBuffer->HeightInPixels)
    {
        DestMaxY = DestinationBuffer->HeightInPixels;
    }

    u32 *SourcePixelsRow =
        SourceBitMap->Pixels +
        SourceOffsetX +
        SourceBitMap->Width * (SourceBitMap->Height - 1 - SourceOffsetY);

    u8 *DestinationRow =
        (u8 *)DestinationBuffer->PixelsMemory +
        DestMinX * DestinationBuffer->BytesPerPixel +
        DestMinY * DestinationBuffer->BytesPerRow;

    for (i32 Y = DestMinY; Y < DestMaxY; Y++)
    {
        u32 *Destination = (u32 *)DestinationRow;
        u32 *Source = SourcePixelsRow;

        for (i32 X = DestMinX; X < DestMaxX; X++)
        {
            f32 SourceAlpha = (f32)((*Source >> 24) & 0xff) / 255.0f * AlphaFactor;
            f32 SourceRed = (f32)((*Source >> 16) & 0xff);
            f32 SourceGreen = (f32)((*Source >> 8) & 0xff);
            f32 SourceBlue = (f32)((*Source >> 0) & 0xff);

            f32 DestinationRed = (f32)((*Destination >> 16) & 0xff);
            f32 DestinationGreen = (f32)((*Destination >> 8) & 0xff);
            f32 DestinationBlue = (f32)((*Destination >> 0) & 0xff);

            f32 ResultRed = SourceAlpha * SourceRed + (1 - SourceAlpha) * DestinationRed;
            f32 ResultGreen = SourceAlpha * SourceGreen + (1 - SourceAlpha) * DestinationGreen;
            f32 ResultBlue = SourceAlpha * SourceBlue + (1 - SourceAlpha) * DestinationBlue;

            *Destination =
                ((u32)(ResultRed + 0.5f) << 16) |
                ((u32)(ResultGreen + 0.5f) << 8) |
                ((u32)(ResultBlue + 0.5f) << 0);

            Destination++;
            Source++;
        }

        DestinationRow += DestinationBuffer->BytesPerRow;
        SourcePixelsRow -= SourceBitMap->Width;
    }
}

void PushRenderPeice
(
    game_state *GameState, render_peice_group *PeiceGroup,
    loaded_bitmap *Bitmap, v3 Offset, v2 BitmapAlignment,
    v4 Color, f32 EntityJumpZCoefficient, v2 RectangleDimensions
)
{
    Assert(PeiceGroup->Count < ArrayCount(PeiceGroup->Peices));

    render_piece *NewRenderPiece = PeiceGroup->Peices + PeiceGroup->Count++;
    NewRenderPiece->Bitmap = Bitmap;
    NewRenderPiece->Offset.XY = V2(Offset.X, -Offset.Y) * GameState->PixelsToMetersRatio - BitmapAlignment;
    NewRenderPiece->Offset.Z = Offset.Z;
    NewRenderPiece->EntityJumpZCoefficient = EntityJumpZCoefficient;
    NewRenderPiece->Dimensions = RectangleDimensions;
    NewRenderPiece->Color = Color;
}

void PushBitmapRenderPiece
(
    game_state *GameState, render_peice_group *PeiceGroup,
    loaded_bitmap *Bitmap, v3 Offset, v2 BitmapAlignment,
    f32 Alpha, f32 EntityJumpZCoefficient
)
{
    PushRenderPeice(GameState, PeiceGroup, Bitmap, Offset, BitmapAlignment, V4(0, 0, 0, Alpha), EntityJumpZCoefficient, V2(0, 0));
}

void PushRectangleRenderPiece
(
    game_state *GameState, render_peice_group *PeiceGroup, v3 Offset,
    v2 RectangleDimensions, v4 Color, f32 EntityJumpZCoefficient
)
{
    PushRenderPeice(GameState, PeiceGroup, 0, Offset, V2(0, 0), Color, EntityJumpZCoefficient, RectangleDimensions);
}

void PushRectangleOutlineRenderPieces
(
    game_state *GameState, render_peice_group *PeiceGroup,
    v3 Offset, v2 RectangleDimensions, v4 Color, f32 EntityJumpZCoefficient
)
{
    f32 Thickness = 0.1f;
    PushRenderPeice
    (
        GameState, PeiceGroup, 0, Offset - V3(0, RectangleDimensions.Y / 2.0f, 0),
        V2(0, 0), Color, EntityJumpZCoefficient, V2(RectangleDimensions.X, Thickness)
    );
    PushRenderPeice
    (
        GameState, PeiceGroup, 0, Offset + V3(0, RectangleDimensions.Y / 2.0f, 0),
        V2(0, 0), Color, EntityJumpZCoefficient, V2(RectangleDimensions.X, Thickness)
    );
    PushRenderPeice
    (
        GameState, PeiceGroup, 0, Offset - V3(RectangleDimensions.X / 2.0f, 0, 0),
        V2(0, 0), Color, EntityJumpZCoefficient, V2(Thickness, RectangleDimensions.Y)
    );
    PushRenderPeice
    (
        GameState, PeiceGroup, 0, Offset + V3(RectangleDimensions.X / 2.0f, 0, 0),
        V2(0, 0), Color, EntityJumpZCoefficient, V2(Thickness, RectangleDimensions.Y)
    );
}

void DrawHitpoints(game_state *GameState, render_peice_group *EntityPeiceGroup, entity *Entity)
{
    if (Entity->HitPointsMax >= 1)
    {
        v2 HitPointDimension = V2(0.2f, 0.2f);
        f32 HitPointXSpacing = 1.5f * HitPointDimension.X;
        v3 HitPointDelta = V3(HitPointXSpacing, 0, 0);
        v3 HitPointOffset = V3(-0.5f * HitPointXSpacing * (Entity->HitPointsMax - 1), -0.25f, 0);

        for (u32 HitPointIndex = 0; HitPointIndex < Entity->HitPointsMax; HitPointIndex++)
        {
            entity_hit_point *HitPoint = Entity->HitPoints + HitPointIndex;
            v4 Color = V4(1.0f, 0, 0, 1.0f);
            if (HitPoint->FilledAmount == 0)
            {
                Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
            }

            PushRectangleRenderPiece
            (
                GameState, EntityPeiceGroup, HitPointOffset, HitPointDimension, Color, 0.0f
            );
            HitPointOffset += HitPointDelta;
        }
    }
}