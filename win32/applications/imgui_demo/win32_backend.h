#pragma once

#include <windows.h>
#include "portable\shared\base_types.h"
#include "win32\third_party\imgui\imgui.h"

#define VK_KEYPAD_ENTER (VK_RETURN + 256)

enum win32_mouse_area
{
    MA_NONE = 0,
    MA_CLIENT_AREA = 1,
    MA_NON_CLIENT_AREA = 2,
};

struct win32_backend_state
{
    HWND Window;
    HWND MouseWindow;
    win32_mouse_area MouseTrackedArea;
    i32 MouseButtonsDown;
    i64 Time;
    i64 TicksPerSecond;
    ImGuiMouseCursor LastMouseCursor;
    b32 MonitorsNeedUpdate;
};

struct win32_viewport_data
{
    HWND Window;
    HWND ParentWindow;
    b32 WindowOwned;
    DWORD StyleFlags;
    DWORD ExtendedStyleFlags;
};

enum win32_renderer_backend
{
    W32RB_OPENGL2,
    W32RB_DX11
};

struct win32_global_handles
{
    HMODULE ShcoreDllModule;
    HMODULE User32DllModule;
    HMODULE NtDllModule;
};

extern win32_global_handles Win32GlobalHandles;

LRESULT Win32_CustomCallbackHandler(HWND Window, u32 Message, WPARAM WParam, LPARAM LParam);
b32 Win32_LoadNecessaryDlls();
b32 Win32_Initialize(void *Window, win32_renderer_backend RendererType);
void Win32_NewFrame();
void Win32_Shutdown();