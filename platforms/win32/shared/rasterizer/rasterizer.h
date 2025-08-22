#pragma once

struct rendering_buffer
{
    BITMAPINFO Bitmapinfo;
    void *Memory;
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
    u32 Pitch;
};

void DrawRectangle(rendering_buffer *Buffer, v2 MinCorner, v2 MaxCorner, v4 Color);
void DrawLine(rendering_buffer *Buffer, v2 Start, v2 End, v4 Color);
void DrawFilledCircle(rendering_buffer *Buffer, v2 CenterPosition, f32 CircleRadius, v4 Color);
void DrawGraph(rendering_buffer *Buffer, f32 *DataPoints, v4 *DataColors, u32 XAxisCount, u32 YAxisRange, rectangle2 GraphRectangle);