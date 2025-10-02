#include <Windows.h>
#include <stdint.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

void DisplayRenderBufferInWindow
(
    HWND Window, HDC DeviceContext,
    void *BufferMemory, u32 BufferWidth, u32 BufferHeight,
    BITMAPINFO *BufferBitmapInfo
)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    i32 WindowWidth = ClientRect.right - ClientRect.left;
    i32 WindowHeight = ClientRect.bottom - ClientRect.top;

    if ((WindowWidth == 1920) && (WindowHeight == 1027))
    {
        i32 DestinationWidth = (i32)(BufferWidth * 1.3);
        i32 DestinationHeight = (i32)(BufferHeight * 1.3);
        StretchDIBits
        (
            DeviceContext,
            0, 0, WindowWidth, WindowHeight,
            0, 0, BufferWidth, BufferHeight,
            BufferMemory, BufferBitmapInfo, DIB_RGB_COLORS, SRCCOPY
        );
    }
    else
    {
        i32 OffsetX = 10;
        i32 OffsetY = 10;

        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, WHITENESS); // top bar
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, WHITENESS); // left bar
        PatBlt(DeviceContext, WindowWidth - OffsetX, 0, OffsetX, WindowHeight, WHITENESS); // right bar
        PatBlt(DeviceContext, 0, WindowHeight - OffsetY, WindowWidth, OffsetY, WHITENESS); // bottom bar

        StretchDIBits
        (
            DeviceContext,
            OffsetX, OffsetY, WindowWidth - 2 * OffsetX, WindowHeight - 2 * OffsetX,
            0, 0, BufferWidth, BufferHeight,
            BufferMemory, BufferBitmapInfo, DIB_RGB_COLORS, SRCCOPY
        );
    }
}

#if 0
// TODO: generic implementation to be used by multiple projects
static v2
GetWindowDimensions(HWND Window)
{
    v2 Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    Result.X = (f32)(ClientRect.right - ClientRect.left);
    Result.Y = (f32)(ClientRect.bottom - ClientRect.top);

    return Result;
}

static void
Win32ToggleFullScreen(HWND Window, win32_platform_state *PlatformState)
{
    u32 CurrentWindowStyle = GetWindowLong(Window, GWL_STYLE);

    if (CurrentWindowStyle & WS_OVERLAPPEDWINDOW)
    {
        HMONITOR Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        b32 GetMonitorInfoResult = GetMonitorInfo(Monitor, &MonitorInfo);

        b32 GetWindowPlacementResult =
            GetWindowPlacement(Window, &PlatformState->PreviousWindowPlacement);

        if (GetWindowPlacementResult && GetMonitorInfoResult)
        {
            SetWindowLong(Window, GWL_STYLE, CurrentWindowStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos
            (
                Window, HWND_TOP,
                MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            );
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, CurrentWindowStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &PlatformState->PreviousWindowPlacement);
        SetWindowPos
        (
            Window, 0, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );
    }
}
#endif