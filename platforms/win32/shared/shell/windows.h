#pragma once

void DisplayRenderBufferInWindow
(
    HWND Window, HDC DeviceContext,
    void *BufferMemory, u32 BufferWidth, u32 BufferHeight,
    BITMAPINFO *BufferBitmapInfo
);