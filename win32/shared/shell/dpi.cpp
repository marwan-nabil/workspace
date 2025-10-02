#include <Windows.h>
#include <stdint.h>
#include <shellscalingapi.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\system\version.h"
#include "dpi.h"

b32 ConfigureDpiAwareness(HMODULE NtDllModule, HMODULE User32DllModule)
{
    if (IsWindowsVersionGreaterOrEqual(NtDllModule, HIBYTE(0x0A00), LOBYTE(0x0A00), 0)) // _WIN32_WINNT_WIN10
    {
        set_thread_dpi_awareness SetThreadDpiAwarenessContextFunction =
            (set_thread_dpi_awareness)
            GetProcAddress(User32DllModule, "SetThreadDpiAwarenessContext");

        if (SetThreadDpiAwarenessContextFunction)
        {
            SetThreadDpiAwarenessContextFunction(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return TRUE;
        }
    }

    if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        return TRUE;
    }

    return FALSE;
}

f32 GetDpiScaleForMonitor(HMODULE NtDllModule, HMODULE ShcoreDllModule, void *Monitor)
{
    u32 DpiX = 96;
    u32 DpiY = 96;

    if (IsWindowsVersionGreaterOrEqual(NtDllModule, HIBYTE(0x0603), LOBYTE(0x0603), 0)) // _WIN32_WINNT_WINBLUE
    {
        get_dpi_for_monitror GetDpiForMonitorFunction = NULL;

        if (ShcoreDllModule)
        {
            GetDpiForMonitorFunction =
                (get_dpi_for_monitror)
                GetProcAddress(ShcoreDllModule, "GetDpiForMonitor");
        }

        if (GetDpiForMonitorFunction)
        {
            GetDpiForMonitorFunction((HMONITOR)Monitor, MDT_EFFECTIVE_DPI, &DpiX, &DpiY);
            Assert(DpiX == DpiY);
            return DpiX / 96.0f;
        }
    }

    HDC DeviceContext = GetDC(NULL);
    DpiX = GetDeviceCaps(DeviceContext, LOGPIXELSX);
    DpiY = GetDeviceCaps(DeviceContext, LOGPIXELSY);
    Assert(DpiX == DpiY);
    ReleaseDC(NULL, DeviceContext);

    return DpiX / 96.0f;
}