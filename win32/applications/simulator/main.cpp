#include <Windows.h>
#include <stdint.h>
#include <math.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\math\rectangle2.h"
#include "win32\shared\rasterizer\rasterizer.h"
#include "win32\shared\shell\windows.h"
#include "win32\shared\system\timing.h"

#include "electrical.h"
#include "logic_gates.h"
#include "internal_types.h"
#include "simulation.h"
#include "rendering.h"

inline void ProcessButton(button_state *Button, b32 IsDown)
{
    if (Button->IsDown != IsDown)
    {
        Button->IsDown = IsDown;
        ++Button->TransitionCount;
    }
}

void ProcessUserInputMessage(MSG Message, user_input *UserInput)
{
    u32 VirtualKeyCode = (u32)Message.wParam;
    b8 KeyWasDown = (Message.lParam & (1 << 30)) != 0;
    b8 KeyIsDown = (Message.lParam & (1ll << 31)) == 0;
    b8 AltKeyIsDown = (Message.lParam & (1 << 29)) != 0;

    if (KeyIsDown != KeyWasDown)
    {
        if (VirtualKeyCode == VK_UP)
        {
            ProcessButton(&UserInput->Up, KeyIsDown);
        }
        else if (VirtualKeyCode == VK_LEFT)
        {
            ProcessButton(&UserInput->Left, KeyIsDown);
        }
        else if (VirtualKeyCode == VK_DOWN)
        {
            ProcessButton(&UserInput->Down, KeyIsDown);
        }
        else if (VirtualKeyCode == VK_RIGHT)
        {
            ProcessButton(&UserInput->Right, KeyIsDown);
        }
        else if (VirtualKeyCode == VK_SHIFT)
        {
            ProcessButton(&UserInput->Shift, KeyIsDown);
        }
    }

    if ((VirtualKeyCode == VK_F4) && KeyIsDown && AltKeyIsDown)
    {
        UserInput->ExitSignal = TRUE;
    }
}

LRESULT CALLBACK
MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CREATE:
        {
            CREATESTRUCT *LocalCreateStruct = (CREATESTRUCT *)LParam;
            SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)LocalCreateStruct->lpCreateParams);
            SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
        } break;

        case WM_PAINT:
        {
            window_private_data *WindowPrivateData = (window_private_data *)GetWindowLongPtr(Window, GWLP_USERDATA);

            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            DisplayRenderBufferInWindow
            (
                Window,
                DeviceContext,
                WindowPrivateData->LocalRenderingBuffer->Memory,
                WindowPrivateData->LocalRenderingBuffer->Width,
                WindowPrivateData->LocalRenderingBuffer->Height,
                &WindowPrivateData->LocalRenderingBuffer->Bitmapinfo
            );
            EndPaint(Window, &Paint);
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(FALSE);
        } break;

        case WM_DESTROY:
        case WM_QUIT:
        case WM_CLOSE:
        {
            window_private_data *WindowPrivateData =
                (window_private_data *)GetWindowLongPtr(Window, GWLP_USERDATA);
            *WindowPrivateData->RunningState = FALSE;
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

i32
WinMain
(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    i32 ShowCmd
)
{
    b32 RunningState = FALSE;

    rendering_buffer LocalRenderingBuffer = {};
    LocalRenderingBuffer.Width = 1920;
    LocalRenderingBuffer.Height = 1080;
    LocalRenderingBuffer.BytesPerPixel = 4;
    LocalRenderingBuffer.Pitch = LocalRenderingBuffer.Width * LocalRenderingBuffer.BytesPerPixel;

    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biSize = sizeof(LocalRenderingBuffer.Bitmapinfo.bmiHeader);
    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biWidth = LocalRenderingBuffer.Width;
    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biHeight = (LONG)LocalRenderingBuffer.Height;
    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biPlanes = 1;
    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biBitCount = 32;
    LocalRenderingBuffer.Bitmapinfo.bmiHeader.biCompression = BI_RGB;

    LocalRenderingBuffer.Memory = VirtualAlloc
    (
        0,
        LocalRenderingBuffer.Width *
        LocalRenderingBuffer.Height *
        LocalRenderingBuffer.BytesPerPixel,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    f32 RendererRefreshHz = 60.0f;
    f32 TargetSecondsPerFrame = 1.0f / RendererRefreshHz;

    i64 WindowsTimerFrequency = GetWindowsTimerFrequency();

    WNDCLASSA MainWindowClass = {};
    MainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    MainWindowClass.lpfnWndProc = MainWindowCallback;
    MainWindowClass.hInstance = Instance;
    MainWindowClass.lpszClassName = "MainWindowClass";
    MainWindowClass.hCursor = LoadCursor(0, IDC_ARROW);

    if (!RegisterClassA(&MainWindowClass))
    {
        return -1;
    }

    window_private_data WindowPrivateData = {};
    WindowPrivateData.LocalRenderingBuffer = &LocalRenderingBuffer;
    WindowPrivateData.RunningState = &RunningState;

    HWND Window = CreateWindowExA
    (
        0,
        MainWindowClass.lpszClassName,
        "simulator",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0,
        0,
        Instance,
        &WindowPrivateData
    );

    if (Window)
    {
        simulation_state SimulationState = {};
        InitializeSimulationState(&SimulationState);

        user_input UserInputBuffers[2] = {};
        user_input *NewUserInput = &UserInputBuffers[0];
        user_input *OldUserInput = &UserInputBuffers[1];

        RunningState = TRUE;
        while (RunningState)
        {
            LARGE_INTEGER FrameStartTime = GetWindowsTimerValue();

            *NewUserInput = {};
            for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewUserInput->Buttons); ButtonIndex++)
            {
                NewUserInput->Buttons[ButtonIndex].IsDown = OldUserInput->Buttons[ButtonIndex].IsDown;
            }

            MSG Message;
            while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
            {
                switch (Message.message)
                {
                    case WM_SYSKEYDOWN:
                    case WM_SYSKEYUP:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    {
                        ProcessUserInputMessage(Message, NewUserInput);
                    } break;

                    default:
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    } break;
                }
            }

            if (NewUserInput->ExitSignal)
            {
                RunningState = FALSE;
            }

            UpdateSimulation(TargetSecondsPerFrame, NewUserInput, &SimulationState);
            RenderSimulation(&LocalRenderingBuffer, &SimulationState);

            HDC DeviceContext = GetDC(Window);
            DisplayRenderBufferInWindow
            (
                Window, DeviceContext,
                LocalRenderingBuffer.Memory,
                LocalRenderingBuffer.Width, LocalRenderingBuffer.Height,
                &LocalRenderingBuffer.Bitmapinfo
            );
            ReleaseDC(Window, DeviceContext);

            user_input *TemporaryUserInput = NewUserInput;
            NewUserInput = OldUserInput;
            OldUserInput = TemporaryUserInput;

            f32 SecondsElapsedForFrame = GetSecondsElapsed(FrameStartTime, GetWindowsTimerValue(), WindowsTimerFrequency);
            if (SecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                DWORD TimeToSleepInMilliSeconds = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                if (TimeToSleepInMilliSeconds > 0)
                {
                    Sleep(TimeToSleepInMilliSeconds);
                }

                do
                {
                    SecondsElapsedForFrame = GetSecondsElapsed(FrameStartTime, GetWindowsTimerValue(), WindowsTimerFrequency);
                } while (SecondsElapsedForFrame < TargetSecondsPerFrame);
            }
        }
    }

    return 0;
}