static void
Win32DebugDrawVertical(win32_pixel_buffer *BackBuffer, i32 X, i32 Top, i32 Bottom, u32 Color)
{
    if (Top <= 0)
    {
        Top = 0;
    }

    if (Bottom >= BackBuffer->HeightInPixels)
    {
        Bottom = BackBuffer->HeightInPixels - 1;
    }

    if ((X >= 0) && (X < BackBuffer->WidthInPixels))
    {
        u8 *Pixel = ((u8 *)BackBuffer->PixelsMemory +
                     X * BackBuffer->BytesPerPixel +
                     Top * BackBuffer->BytesPerRow);

        for (i32 Y = Top;
             Y < Bottom;
             Y++)
        {
            *(u32 *)Pixel = Color;
            Pixel += BackBuffer->BytesPerRow;
        }
    }
}

static void
Win32DrawSoundBufferMarker
(
    win32_pixel_buffer *BackBuffer, win32_sound_configuration *SoundBufferState,
    f32 SoundBufferCoefficient, i32 PadX, i32 Top, i32 Bottom, u32 SoundBufferIndex, u32 Color
)
{
    i32 X = PadX + (i32)(SoundBufferCoefficient * (f32)SoundBufferIndex);
    Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);
}

static void
Win32DebugSyncDisplay
(
    win32_pixel_buffer *BackBuffer, win32_debug_sound_time_marker *TimeMarkers, i32 MarkersCount,
    win32_sound_configuration *SoundBufferConfiguration, f32 TargetSecondsPerFrame, i32 CurrentMarkerIndex
)
{
    i32 PadX = 16;
    i32 PadY = 16;
    i32 LineHeight = 64;

    // AA BB GG RR
    u32 FlipPlayColor = 0xffffffff; // white
    u32 FlipWriteColor = 0xff000000; // black
    u32 OutputPlayColor = 0xffFA90FF; // magenta
    u32 OutputWriteColor = 0xff653468; // dark magenta
    u32 ByteToLockColor = 0xff2AFF00; // green
    u32 TargetCursorColor = 0xff116600; // dark green
    u32 ExpectedFlipColor = 0xffffff00; // yellow

    f32 SoundBufferCoefficient = (f32)(BackBuffer->WidthInPixels - 2 * PadX) / (f32)SoundBufferConfiguration->SoundBufferSize;

    for (int MarkerIndex = 0;
         MarkerIndex < MarkersCount;
         MarkerIndex++)
    {
        win32_debug_sound_time_marker *CurrentMarker = &TimeMarkers[MarkerIndex];
        Assert(CurrentMarker->OutputPlayCursor < SoundBufferConfiguration->SoundBufferSize);
        Assert(CurrentMarker->OutputWriteCursor < SoundBufferConfiguration->SoundBufferSize);
        Assert(CurrentMarker->OutputLocation < SoundBufferConfiguration->SoundBufferSize);
        Assert(CurrentMarker->OutputByteCount < SoundBufferConfiguration->SoundBufferSize);
        Assert(CurrentMarker->FlipPlayCursor < SoundBufferConfiguration->SoundBufferSize);
        Assert(CurrentMarker->FlipWriteCursor < SoundBufferConfiguration->SoundBufferSize);

        i32 Top = PadY;
        i32 Bottom = PadY + LineHeight;

        Win32DrawSoundBufferMarker(BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                                   CurrentMarker->FlipPlayCursor, FlipPlayColor);
                                   //CurrentMarker->FlipPlayCursor + 480 * SoundBufferConfiguration->BytesPerSample, FlipPlayColor);
        Win32DrawSoundBufferMarker(BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                                   CurrentMarker->FlipWriteCursor, FlipWriteColor);

        if (MarkerIndex == CurrentMarkerIndex)
        {
            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;
            i32 FirstTop = Top;

            Win32DrawSoundBufferMarker
            (
                BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                CurrentMarker->OutputPlayCursor, OutputPlayColor
            );
            Win32DrawSoundBufferMarker
            (
                BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                CurrentMarker->OutputWriteCursor, OutputWriteColor
            );

            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            Win32DrawSoundBufferMarker
            (
                BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                CurrentMarker->OutputLocation, ByteToLockColor
            );
            Win32DrawSoundBufferMarker
            (
                BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, Top, Bottom,
                CurrentMarker->OutputLocation + CurrentMarker->OutputByteCount, TargetCursorColor
            );

            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            Win32DrawSoundBufferMarker
            (
                BackBuffer, SoundBufferConfiguration, SoundBufferCoefficient, PadX, FirstTop, Bottom,
                CurrentMarker->ExpectedFlipCursor, ExpectedFlipColor
            );
        }
    }
}