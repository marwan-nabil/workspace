#include <Windows.h>
#include <stdint.h>
#include <math.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\math\rectangle2.h"
#include "rasterizer.h"

#if 0
// TODO: a generic implmentation to be used from multiple projects
static void
ResizeRenderingBuffer(rendering_buffer *Buffer, i32 NewWidthInPixels, i32 NewHeightInPixels)
{
    Buffer->WidthInPixels = NewWidthInPixels;
    Buffer->HeightInPixels = NewHeightInPixels;
    Buffer->BytesPerPixel = 4;
    Buffer->BytesPerRow = Buffer->WidthInPixels * Buffer->BytesPerPixel;

    Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
    Buffer->BitmapInfo.bmiHeader.biWidth = Buffer->WidthInPixels;
    Buffer->BitmapInfo.bmiHeader.biHeight = -Buffer->HeightInPixels;
    Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
    Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
    Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

    if (Buffer->PixelsMemory)
    {
        VirtualFree(Buffer->PixelsMemory, 0, MEM_RELEASE);
    }

    i32 BitmapMemorySize = Buffer->WidthInPixels * Buffer->HeightInPixels * Buffer->BytesPerPixel;
    Buffer->PixelsMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    // NOTE: clear bitmap memory to black
}
#endif

void DrawRectangle(rendering_buffer *Buffer, v2 MinCorner, v2 MaxCorner, v4 Color)
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
    if (MaxX > (i32)Buffer->Width)
    {
        MaxX = (i32)Buffer->Width;
    }
    if (MaxY > (i32)Buffer->Height)
    {
        MaxY = (i32)Buffer->Height;
    }

    u32 ColorU32 = PackColor(Color);

    u8 *Row =
        (u8 *)Buffer->Memory +
        MinX * Buffer->BytesPerPixel +
        MinY * Buffer->Pitch;

    for (i32 Y = MinY; Y < MaxY; Y++)
    {
        u32 *Pixel = (u32 *)Row;
        for (i32 X = MinX; X < MaxX; X++)
        {
            f32 SourceAlpha = (f32)((ColorU32 >> 24) & 0xff) / 255.0f;
            f32 SourceRed = (f32)((ColorU32 >> 16) & 0xff);
            f32 SourceGreen = (f32)((ColorU32 >> 8) & 0xff);
            f32 SourceBlue = (f32)((ColorU32 >> 0) & 0xff);

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
        Row += Buffer->Pitch;
    }
}

void DrawLine(rendering_buffer *Buffer, v2 Start, v2 End, v4 Color)
{
    rectangle2 BufferRectangle = RectangleFromMinMax(V2(0, 0), V2((f32)Buffer->Width, (f32)Buffer->Height));

    Start = ClampPointToRectangle(Start, BufferRectangle);
    End = ClampPointToRectangle(End, BufferRectangle);

    u32 ColorU32 = PackColor(Color);

    i32 StartX = RoundF32ToI32(Start.X);
    i32 StartY = RoundF32ToI32(Start.Y);
    i32 EndX = RoundF32ToI32(End.X);
    i32 EndY = RoundF32ToI32(End.Y);

    i32 MinX = Min(StartX, EndX);
    i32 MinY = Min(StartY, EndY);
    i32 MaxX = Max(StartX, EndX);
    i32 MaxY = Max(StartY, EndY);

    i32 XDiff = EndX - StartX;
    i32 YDiff = EndY - StartY;

    if ((XDiff == 0) && (YDiff == 0))
    {
        u32 *Pixel = (u32 *)((u8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch);
        *Pixel = ColorU32;
    }
    else if (AbsoluteValue(XDiff) > AbsoluteValue(YDiff))
    {
        f32 Slope = (f32)YDiff / (f32)XDiff;
        for (i32 X = MinX; X <= MaxX; X++)
        {
            i32 Y = RoundF32ToI32((f32)StartY + Slope * (X - StartX));
            u32 *Pixel = (u32 *)((u8 *)Buffer->Memory + X * Buffer->BytesPerPixel + Y * Buffer->Pitch);
            *Pixel = ColorU32;
        }
    }
    else
    {
        f32 Slope = (f32)XDiff / (f32)YDiff;
        for (i32 Y = MinY; Y <= MaxY; Y++)
        {
            i32 X = RoundF32ToI32((f32)StartX + Slope * (Y - StartY));
            u32 *Pixel = (u32 *)((u8 *)Buffer->Memory + X * Buffer->BytesPerPixel + Y * Buffer->Pitch);
            *Pixel = ColorU32;
        }
    }
}

void DrawFilledCircle(rendering_buffer *Buffer, v2 CenterPosition, f32 CircleRadius, v4 Color)
{
    i32 MinX = RoundF32ToI32(CenterPosition.X - CircleRadius);
    i32 MinY = RoundF32ToI32(CenterPosition.Y - CircleRadius);
    i32 MaxX = RoundF32ToI32(CenterPosition.X + CircleRadius);
    i32 MaxY = RoundF32ToI32(CenterPosition.Y + CircleRadius);

    if (MinX < 0)
    {
        MinX = 0;
    }
    if (MinY < 0)
    {
        MinY = 0;
    }
    if (MaxX > (i32)Buffer->Width)
    {
        MaxX = (i32)Buffer->Width;
    }
    if (MaxY > (i32)Buffer->Height)
    {
        MaxY = (i32)Buffer->Height;
    }

    u32 ColorU32 = PackColor(Color);

    u8 *DestinationRow = ((u8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch);

    for (i32 Y = MinY; Y < MaxY; Y++)
    {
        u32 *Pixel = (u32 *)DestinationRow;

        for (i32 X = MinX; X < MaxX; X++)
        {
            f32 PointCenterRelativeX = (f32)(X - MinX) - CircleRadius;
            f32 PointCenterRelativeY = (f32)(Y - MinY) - CircleRadius;

            if (Length(v2{PointCenterRelativeX, PointCenterRelativeY}) <= CircleRadius)
            {
                *Pixel = ColorU32;
            }

            Pixel++;
        }

        DestinationRow += Buffer->Pitch;
    }
}

void DrawGraph(rendering_buffer *Buffer, f32 *DataPoints, v4 *DataColors, u32 XAxisCount, u32 YAxisRange, rectangle2 GraphRectangle)
{
    DrawRectangle(Buffer, GraphRectangle.MinPoint, GraphRectangle.MaxPoint, V4(1.0f, 1.0f, 1.0f, 1.0f));

    f32 XPadding = 10;
    f32 YPadding = 10;

    f32 XStepSize = (GraphRectangle.MaxPoint.X - GraphRectangle.MinPoint.X - 2 * XPadding) / XAxisCount;
    f32 XAxisLength = XStepSize * XAxisCount;

    v2 XAxisStartPoint = V2(GraphRectangle.MinPoint.X + XPadding, GraphRectangle.MinPoint.Y + YPadding);
    v2 XAxisEndPoint = V2(XAxisStartPoint.X + XAxisLength, XAxisStartPoint.Y);
    DrawLine(Buffer, XAxisStartPoint, XAxisEndPoint, V4(0, 0, 0, 1.0f));

    f32 YStepSize = (GraphRectangle.MaxPoint.Y - GraphRectangle.MinPoint.Y - 2 * YPadding) / YAxisRange;
    f32 YAxisLength = YStepSize * YAxisRange;

    v2 YAxisStartPoint = XAxisStartPoint;
    v2 YAxisEndPoint = V2(YAxisStartPoint.X, YAxisStartPoint.Y + YAxisLength);
    DrawLine(Buffer, YAxisStartPoint, YAxisEndPoint, V4(0, 0, 0, 1.0f));

    for (u32 DataPointIndex = 0; DataPointIndex < XAxisCount; DataPointIndex++)
    {
        f32 PointX = GraphRectangle.MinPoint.X + XPadding + DataPointIndex * XStepSize;
        f32 PointY = GraphRectangle.MinPoint.Y + YPadding + DataPoints[DataPointIndex] * YStepSize;
        DrawFilledCircle(Buffer, V2(PointX, PointY), 15.0f, DataColors[DataPointIndex]);
    }
}