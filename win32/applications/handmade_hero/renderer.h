#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\handmade_hero\game_interface.h"
#include "win32\handmade_hero\bitmap.h"

struct render_piece
{
    loaded_bitmap *Bitmap;
    v4 Color;
    v3 Offset;
    f32 EntityJumpZCoefficient;
    v2 Dimensions;
};

struct render_peice_group
{
    u32 Count;
    render_piece Peices[8];
};

struct game_state;
struct entity;

void DrawRectangle
(
    game_pixel_buffer *PixelBuffer,
    v2 MinCorner, v2 MaxCorner,
    f32 Red, f32 Green, f32 Blue, f32 Alpha
);

void DrawBitmap
(
    loaded_bitmap *SourceBitMap, game_pixel_buffer *DestinationBuffer,
    f32 DestinationX, f32 DestinationY, f32 AlphaFactor
);

void PushRenderPeice
(
    game_state *GameState, render_peice_group *PeiceGroup,
    loaded_bitmap *Bitmap, v3 Offset, v2 BitmapAlignment,
    v4 Color, f32 EntityJumpZCoefficient, v2 RectangleDimensions
);

void PushBitmapRenderPiece
(
    game_state *GameState, render_peice_group *PeiceGroup,
    loaded_bitmap *Bitmap, v3 Offset, v2 BitmapAlignment,
    f32 Alpha, f32 EntityJumpZCoefficient
);

void PushRectangleRenderPiece
(
    game_state *GameState, render_peice_group *PeiceGroup, v3 Offset,
    v2 RectangleDimensions, v4 Color, f32 EntityJumpZCoefficient
);

void PushRectangleOutlineRenderPieces
(
    game_state *GameState, render_peice_group *PeiceGroup,
    v3 Offset, v2 RectangleDimensions, v4 Color, f32 EntityJumpZCoefficient
);

void DrawHitpoints(game_state *GameState, render_peice_group *EntityPeiceGroup, entity *Entity);