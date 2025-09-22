#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <Xinput.h>
#include <dsound.h>
#include <intrin.h>
#include <stdio.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\strings\strings.h"
#include "game_interface.h"
#include "win32_platform.h"

static win32_platform_state Win32PlatformState = {};
static xinput_get_state *XInputGetState_ = XInputGetStateStub;
static xinput_set_state *XInputSetState_ = XInputSetStateStub;

XINPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

XINPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

static void
Win32GetEXEFilePath(win32_platform_state *PlatformState)
{
    u32 ExeFilePathSize =
        GetModuleFileNameA
        (
            NULL,
            PlatformState->ExeFilePath,
            sizeof(PlatformState->ExeFilePath)
        );

    PlatformState->ExeFilePathOnePastLastSlash = PlatformState->ExeFilePath;

    for
    (
        char *ScanedCharacter = PlatformState->ExeFilePath;
        *ScanedCharacter;
        ScanedCharacter++
    )
    {
        if (*ScanedCharacter == '\\')
        {
            PlatformState->ExeFilePathOnePastLastSlash = ScanedCharacter + 1;
        }
    }
}

static void
Win32BuildFilePath(win32_platform_state *PlatformState, char *FileName, u32 DestinationBufferSize, char *DestinationBuffer)
{
    size_t FirstStringLength = PlatformState->ExeFilePathOnePastLastSlash - PlatformState->ExeFilePath;
    size_t SecondStringLength = strlen(FileName);
    Assert((FirstStringLength + SecondStringLength) < DestinationBufferSize);

    memcpy(DestinationBuffer, PlatformState->ExeFilePath, FirstStringLength);
    memcpy(DestinationBuffer + FirstStringLength, FileName, SecondStringLength);
    DestinationBuffer[FirstStringLength + SecondStringLength] = '\0';
}

PLATFORM_FREE_FILE_MEMORY(PlatformFreeFileMemory)
{
    if (FileMemory)
    {
        VirtualFree(FileMemory, 0, MEM_RELEASE);
    }
}

PLATFORM_READ_FILE(PlatformReadFile)
{
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            void *FileMemory = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (FileMemory)
            {
                DWORD BytesRead;
                DWORD FileSizeU32 = SafeTruncateUint64ToUint32(FileSize.QuadPart);
                if
                (
                    ReadFile(FileHandle, FileMemory, FileSizeU32, &BytesRead, 0) &&
                    (FileSizeU32 == BytesRead)
                )
                {
                    Result.FileContents = FileMemory;
                    Result.ConstentsSize = FileSizeU32;
                }
                else
                {
                    PlatformFreeFileMemory(Thread, FileMemory);
                }
            }
        }

        CloseHandle(FileHandle);
    }

    return Result;
}

PLATFORM_WRITE_FILE(PlatformWriteFile)
{
    b32 Result = FALSE;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, DataToWrite, DataSize, &BytesWritten, 0))
        {
            Result = (DataSize == BytesWritten);
        }

        CloseHandle(FileHandle);
    }

    return Result;
}

static FILETIME
Win32GetFileLastWriteTime(char *FileName)
{
    FILETIME Result = {};

    WIN32_FILE_ATTRIBUTE_DATA FileAttributes;
    if (GetFileAttributesExA(FileName, GetFileExInfoStandard, &FileAttributes))
    {
        Result = FileAttributes.ftLastWriteTime;
    }

    return Result;
}

static win32_game_code
Win32LoadGameCode(char *GameCodeSourceDLLPath, char *GameCodeTempDLLPath)
{
    win32_game_code Result = {};

    CopyFileA(GameCodeSourceDLLPath, GameCodeTempDLLPath, FALSE);

    Result.LastWriteTimeForLoadedDLL = Win32GetFileLastWriteTime(GameCodeTempDLLPath);
    Result.GameCodeDll = LoadLibraryA(GameCodeTempDLLPath);

    if (Result.GameCodeDll)
    {
        Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.GameCodeDll, "GameUpdateAndRender");
        Result.GetSoundSamples = (game_get_sound_samples *)GetProcAddress(Result.GameCodeDll, "GameGetSoundSamples");
        Result.IsValid = Result.UpdateAndRender && Result.GetSoundSamples;
    }

    if (!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }

    return Result;
}

static void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if (GameCode->GameCodeDll)
    {
        FreeLibrary(GameCode->GameCodeDll);
    }

    GameCode->IsValid = FALSE;
    GameCode->GameCodeDll = NULL;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
    GameCode->LastWriteTimeForLoadedDLL = {};
}

static void
Win32LoadXInput()
{
    HMODULE XInputDLL = LoadLibraryA("xinput1_4.dll");
    if (!XInputDLL)
    {
        XInputDLL = LoadLibraryA("xinput1_3.dll");
    }
    if (!XInputDLL)
    {
        XInputDLL = LoadLibraryA("xinput9_1_0.dll");
    }

    if (XInputDLL)
    {
        XInputGetState_ = (xinput_get_state *)GetProcAddress(XInputDLL, "XInputGetState");
        if (!XInputGetState_)
        {
            XInputGetState_ = XInputGetStateStub;
        }

        XInputSetState_ = (xinput_set_state *)GetProcAddress(XInputDLL, "XInputSetState");
        if (!XInputSetState_)
        {
            XInputSetState_ = XInputSetStateStub;
        }
    }
}

static void
Win32InitDSound(HWND Window, win32_sound_configuration *SoundBufferConfiguratoin, LPDIRECTSOUNDBUFFER *SecondarySoundBuffer)
{
    HMODULE DSoundDLL = LoadLibraryA("dsound.dll");
    if (DSoundDLL)
    {
        directsound_create *DirectSoundCreate =
            (directsound_create *)GetProcAddress(DSoundDLL, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SoundBufferConfiguratoin->SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                LPDIRECTSOUNDBUFFER PrimarySoundBuffer;
                DSBUFFERDESC PrimaryBufferDescription = {};
                PrimaryBufferDescription.dwSize = sizeof(PrimaryBufferDescription);
                PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription, &PrimarySoundBuffer, 0)))
                {
                    if (SUCCEEDED(PrimarySoundBuffer->SetFormat(&WaveFormat)))
                    {
                        OutputDebugStringA("INFO: Primary Sound Buffer Created And Fromat Set!\n");
                    }
                }
            }

            DSBUFFERDESC SecondaryBufferDescription = {};
            SecondaryBufferDescription.dwSize = sizeof(SecondaryBufferDescription);
            SecondaryBufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            SecondaryBufferDescription.dwBufferBytes = SoundBufferConfiguratoin->SoundBufferSize;
            SecondaryBufferDescription.lpwfxFormat = &WaveFormat;

            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, SecondarySoundBuffer, 0)))
            {
                OutputDebugStringA("INFO: secondary Sound Buffer Created!\n");
            }
        }
    }
}

static void
Win32ResizePixelBuffer(win32_pixel_buffer *PixelBuffer, i32 NewWidthInPixels, i32 NewHeightInPixels)
{
    PixelBuffer->WidthInPixels = NewWidthInPixels;
    PixelBuffer->HeightInPixels = NewHeightInPixels;
    PixelBuffer->BytesPerPixel = 4;
    PixelBuffer->BytesPerRow = PixelBuffer->WidthInPixels * PixelBuffer->BytesPerPixel;

    PixelBuffer->BitmapInfo.bmiHeader.biSize = sizeof(PixelBuffer->BitmapInfo.bmiHeader);
    PixelBuffer->BitmapInfo.bmiHeader.biWidth = PixelBuffer->WidthInPixels;
    PixelBuffer->BitmapInfo.bmiHeader.biHeight = -PixelBuffer->HeightInPixels;
    PixelBuffer->BitmapInfo.bmiHeader.biPlanes = 1;
    PixelBuffer->BitmapInfo.bmiHeader.biBitCount = 32;
    PixelBuffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

    if (PixelBuffer->PixelsMemory)
    {
        VirtualFree(PixelBuffer->PixelsMemory, 0, MEM_RELEASE);
    }

    i32 BitmapMemorySize = PixelBuffer->WidthInPixels * PixelBuffer->HeightInPixels * PixelBuffer->BytesPerPixel;
    PixelBuffer->PixelsMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static void
Win32DisplayBufferInWindow(HDC DeviceContext, win32_window_dimensions WindowDimensions, win32_pixel_buffer *PixelBuffer)
{
    if ((WindowDimensions.WidthInPixels >= 1280) && (WindowDimensions.HeightInPixels >= 800))
    {
        i32 DestinationWidth = (i32)(PixelBuffer->WidthInPixels * 1.3);
        i32 DestinationHeight = (i32)(PixelBuffer->HeightInPixels * 1.3);

        StretchDIBits
        (
            DeviceContext,
            0, 0, DestinationWidth, DestinationHeight,
            0, 0, PixelBuffer->WidthInPixels, PixelBuffer->HeightInPixels,
            PixelBuffer->PixelsMemory, &PixelBuffer->BitmapInfo, DIB_RGB_COLORS, SRCCOPY
        );
    }
    else
    {
        i32 OffsetX = 10;
        i32 OffsetY = 10;

        PatBlt(DeviceContext, 0, 0, WindowDimensions.WidthInPixels, OffsetY, BLACKNESS);
        PatBlt
        (
            DeviceContext,
            0, OffsetY + PixelBuffer->HeightInPixels,
            WindowDimensions.WidthInPixels, WindowDimensions.HeightInPixels,
            BLACKNESS
        );
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowDimensions.HeightInPixels, BLACKNESS);
        PatBlt
        (
            DeviceContext,
            OffsetX + PixelBuffer->WidthInPixels, 0,
            WindowDimensions.WidthInPixels, WindowDimensions.HeightInPixels,
            BLACKNESS
        );

        StretchDIBits
        (
            DeviceContext,
            OffsetX, OffsetY, PixelBuffer->WidthInPixels, PixelBuffer->HeightInPixels,
            0, 0, PixelBuffer->WidthInPixels, PixelBuffer->HeightInPixels,
            PixelBuffer->PixelsMemory, &PixelBuffer->BitmapInfo, DIB_RGB_COLORS, SRCCOPY
        );
    }
}

static win32_window_dimensions
Win32GetWindowDimensions(HWND Window)
{
    win32_window_dimensions Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    Result.WidthInPixels = ClientRect.right - ClientRect.left;
    Result.HeightInPixels = ClientRect.bottom - ClientRect.top;

    return Result;
}

static void
Win32FillGlobalSoundBuffer
(
    win32_sound_configuration *SoundBufferConfiguration,
    win32_sound_state *SoundBufferState,
    DWORD ByteToLock,
    DWORD BytesToWrite,
    LPDIRECTSOUNDBUFFER SecondarySoundBuffer,
    game_sound_request *GameSoundBuffer
)
{
    void *BufferRegion1;
    DWORD BufferRegion1Size;
    void *BufferRegion2;
    DWORD BufferRegion2Size;

    if
    (
        SUCCEEDED
        (
            SecondarySoundBuffer->Lock
            (
                ByteToLock, BytesToWrite,
                &BufferRegion1, &BufferRegion1Size,
                &BufferRegion2, &BufferRegion2Size,
                0
            )
        )
    )
    {
        i16 *SourceHalfSample = GameSoundBuffer->OutputSamples;
        i16 *DestinationHalfSample = (i16 *)BufferRegion1;
        u32 Region1SampleCount = BufferRegion1Size / SoundBufferConfiguration->BytesPerSample;

        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
        {
            *DestinationHalfSample++ = *SourceHalfSample++;
            *DestinationHalfSample++ = *SourceHalfSample++;
            SoundBufferState->RunningSampleIndex++;
        }

        DestinationHalfSample = (i16 *)BufferRegion2;
        u32 Region2SampleCount = BufferRegion2Size / SoundBufferConfiguration->BytesPerSample;

        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
        {
            *DestinationHalfSample++ = *SourceHalfSample++;
            *DestinationHalfSample++ = *SourceHalfSample++;
            SoundBufferState->RunningSampleIndex++;
        }

        SecondarySoundBuffer->Unlock(BufferRegion1, BufferRegion1Size, BufferRegion2, BufferRegion2Size);
    }
}

static void
Win32ClearSoundBuffer(win32_sound_configuration *SoundBufferConfiguration, LPDIRECTSOUNDBUFFER SecondarySoundBuffer)
{
    void *BufferRegion1;
    DWORD BufferRegion1Size;
    void *BufferRegion2;
    DWORD BufferRegion2Size;

    if
    (
        SUCCEEDED
        (
            SecondarySoundBuffer->Lock
            (
                0, SoundBufferConfiguration->SoundBufferSize,
                &BufferRegion1, &BufferRegion1Size,
                &BufferRegion2, &BufferRegion2Size,
                0
            )
        )
    )
    {
        u8 *DestinationByte = (u8 *)BufferRegion1;
        for (DWORD ByteIndex = 0; ByteIndex < BufferRegion1Size; ByteIndex++)
        {
            *DestinationByte++ = 0;
        }

        DestinationByte = (u8 *)BufferRegion2;
        for (DWORD ByteIndex = 0; ByteIndex < BufferRegion2Size; ByteIndex++)
        {
            *DestinationByte++ = 0;
        }

        SecondarySoundBuffer->Unlock(BufferRegion1, BufferRegion1Size, BufferRegion2, BufferRegion2Size);
    }
}

static void
Win32ProcessButton(button_state *NewButtonState, b32 IsDown)
{
    if (NewButtonState->IsDown != IsDown)
    {
        NewButtonState->IsDown = IsDown;
        ++NewButtonState->TransitionsCount;
    }
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_ACTIVATEAPP:
        {
            u8 Alpha = 255;
            if (WParam == FALSE)
            {
               Alpha = 128;
            }
            //SetLayeredWindowAttributes(MainWindow, RGB(0, 0, 0), Alpha, LWA_ALPHA);
        } break;

        case WM_SIZE:
        {
            OutputDebugStringA("INFO: WM_SIZE\n");
        } break;

        case WM_SETCURSOR:
        {
            if (Win32PlatformState.ShowCursor)
            {
                DefWindowProcA(Window, Message, WParam, LParam);
            }
            else
            {
                SetCursor(0);
            }
        } break;

        case WM_PAINT:
        {
            OutputDebugStringA("INFO: WM_PAINT\n");
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            Win32DisplayBufferInWindow(DeviceContext, Win32GetWindowDimensions(Window), &Win32PlatformState.BackBuffer);
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
        {
            OutputDebugStringA("INFO: WM_DESTROY\n");
            Win32PlatformState.IsRunning = FALSE;
        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("INFO: WM_CLOSE\n");
            Win32PlatformState.IsRunning = FALSE;
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

static f32
Win32ProcessStickDeadzone(SHORT StickValueIn, SHORT DeadzoneThreshold)
{
    f32 StickValueOut;

    if (StickValueIn < -DeadzoneThreshold)
    {
        StickValueOut = (f32)(StickValueIn + DeadzoneThreshold) / (32768.0f - (f32)DeadzoneThreshold);
    }
    else if (StickValueIn > DeadzoneThreshold)
    {
        StickValueOut = (f32)(StickValueIn - DeadzoneThreshold) / (32767.0f - (f32)DeadzoneThreshold);
    }
    else
    {
        StickValueOut = 0.0f;
    }

    return StickValueOut;
}

static void
Win32CaptureGamepadControllerSample
(
    controller_state *NewControllerState,
    controller_state *OldControllerState,
    XINPUT_STATE *SampledControllerState
)
{
    NewControllerState->IsConnected = TRUE;
    NewControllerState->MovementIsAnalog = OldControllerState->MovementIsAnalog;

    XINPUT_GAMEPAD *GamePadSample = &SampledControllerState->Gamepad;

    NewControllerState->StickAverageX =
        Win32ProcessStickDeadzone(GamePadSample->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    NewControllerState->StickAverageY =
        Win32ProcessStickDeadzone(GamePadSample->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

    if
    (
        (NewControllerState->StickAverageX != 0.0f) ||
        (NewControllerState->StickAverageY != 0.0f)
    )
    {
        NewControllerState->MovementIsAnalog = TRUE;
    }

    if (GamePadSample->wButtons & XINPUT_GAMEPAD_DPAD_UP)
    {
        NewControllerState->StickAverageY = 1.0f;
        NewControllerState->MovementIsAnalog = FALSE;
    }
    if (GamePadSample->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
    {
        NewControllerState->StickAverageY = -1.0f;
        NewControllerState->MovementIsAnalog = FALSE;
    }
    if (GamePadSample->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
    {
        NewControllerState->StickAverageX = 1.0f;
        NewControllerState->MovementIsAnalog = FALSE;
    }
    if (GamePadSample->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
    {
        NewControllerState->StickAverageX = 1.0f;
        NewControllerState->MovementIsAnalog = FALSE;
    }

    f32 AnalogStickDigitalThreshold = 0.5f;

    Win32ProcessButton
    (
        &NewControllerState->MoveLeft,
        ((NewControllerState->StickAverageX < -AnalogStickDigitalThreshold) ? 1 : 0)
    );
    Win32ProcessButton
    (
        &NewControllerState->MoveRight,
        ((NewControllerState->StickAverageX > AnalogStickDigitalThreshold) ? 1 : 0)
    );
    Win32ProcessButton
    (
        &NewControllerState->MoveUp,
        ((NewControllerState->StickAverageY > AnalogStickDigitalThreshold) ? 1 : 0)
    );
    Win32ProcessButton
    (
        &NewControllerState->MoveDown,
        ((NewControllerState->StickAverageY < -AnalogStickDigitalThreshold) ? 1 : 0)
    );

    Win32ProcessButton
    (
        &NewControllerState->ActionUp,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y)
    );
    Win32ProcessButton
    (
        &NewControllerState->ActionDown,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A)
    );
    Win32ProcessButton
    (
        &NewControllerState->ActionLeft,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X)
    );
    Win32ProcessButton
    (
        &NewControllerState->ActionRight,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B)
    );
    Win32ProcessButton
    (
        &NewControllerState->LeftShoulder,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) == XINPUT_GAMEPAD_LEFT_SHOULDER)
    );
    Win32ProcessButton
    (
        &NewControllerState->RightShoulder,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == XINPUT_GAMEPAD_RIGHT_SHOULDER)
    );
    Win32ProcessButton
    (
        &NewControllerState->Start,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START)
    );
    Win32ProcessButton
    (
        &NewControllerState->Back,
        ((GamePadSample->wButtons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK)
    );
}

static void
Win32GetInputOrStateFileLocation
(
    i32 SlotIndex,
    b32 IsInputStream,
    win32_platform_state *PlatformState,
    u32 DestSize,
    char *Dest
)
{
    char Temp[64];
    _snprintf_s
    (
        Temp,
        ArrayCount(Temp),
        "loop_edit_%d_%s.hmi",
        SlotIndex,
        IsInputStream ? "input" : "state"
    );
    Win32BuildFilePath(PlatformState, Temp, DestSize, Dest);
}

static win32_input_buffer *
Win32GetGameInputBuffer(win32_platform_state *PlatformState, u32 Index)
{
    Assert(Index > 0);
    Assert(Index < ArrayCount(PlatformState->GameInputBuffers));

    win32_input_buffer *GameInputBuffer = &PlatformState->GameInputBuffers[Index];
    return GameInputBuffer;
}

static void
Win32BeginRecordingInput(win32_platform_state *PlatformState, u32 CurrentGameInputRecordingBufferIndex)
{
    win32_input_buffer *GameInputBuffer =
        Win32GetGameInputBuffer(PlatformState, CurrentGameInputRecordingBufferIndex);

    if (GameInputBuffer->MemoryBlock)
    {
        PlatformState->CurrentGameInputRecordingBufferIndex =
            CurrentGameInputRecordingBufferIndex;

        char GameInputRecordingFileName[MAX_PATH];
        Win32GetInputOrStateFileLocation
        (
            CurrentGameInputRecordingBufferIndex, TRUE, PlatformState,
            sizeof(GameInputRecordingFileName), GameInputRecordingFileName
        );

        PlatformState->CurrentGameInputRecordingFileHandle =
            CreateFileA(GameInputRecordingFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

        CopyMemory(GameInputBuffer->MemoryBlock, PlatformState->GameMemoryBlock, PlatformState->GameMemoryBlockSize);
    }
}

static void
Win32EndRecordingInput(win32_platform_state *PlatformState)
{
    PlatformState->CurrentGameInputRecordingBufferIndex = 0;
    CloseHandle(PlatformState->CurrentGameInputRecordingFileHandle);
    PlatformState->CurrentGameInputRecordingFileHandle = NULL;
}

static void
Win32BeginInputPlayback(win32_platform_state *PlatformState, u32 CurrentGameInputPlaybackBufferIndex)
{
    win32_input_buffer *GameInputBuffer =
        Win32GetGameInputBuffer(PlatformState, CurrentGameInputPlaybackBufferIndex);

    if (GameInputBuffer->MemoryBlock)
    {
        PlatformState->CurrentGameInputPlaybackBufferIndex =
            CurrentGameInputPlaybackBufferIndex;

        char CurrentGameInputPlaybackFileName[MAX_PATH];
        Win32GetInputOrStateFileLocation
        (
            CurrentGameInputPlaybackBufferIndex, TRUE, PlatformState,
            sizeof(CurrentGameInputPlaybackFileName), CurrentGameInputPlaybackFileName
        );

        PlatformState->CurrentInputPlaybackFileHandle =
            CreateFileA(CurrentGameInputPlaybackFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

        CopyMemory(PlatformState->GameMemoryBlock, GameInputBuffer->MemoryBlock, PlatformState->GameMemoryBlockSize);
    }
}

static void
Win32EndInputPlayback(win32_platform_state *PlatformState)
{
    PlatformState->CurrentGameInputPlaybackBufferIndex = 0;
    CloseHandle(PlatformState->CurrentInputPlaybackFileHandle);
}

static void
Win32RecordInput(win32_platform_state *PlatformState, game_input *GameInput)
{
    DWORD BytesWritten;
    WriteFile
    (
        PlatformState->CurrentGameInputRecordingFileHandle, GameInput,
        sizeof(game_input), &BytesWritten, 0
    );
}

static void
Win32PlaybackInput(win32_platform_state *PlatformState, game_input *GameInput)
{
    DWORD BytesRead;
    if
    (
        ReadFile
        (
            PlatformState->CurrentInputPlaybackFileHandle, GameInput,
            sizeof(game_input), &BytesRead, 0
        )
    )
    {
        if (BytesRead == 0)
        {
            Win32EndInputPlayback(PlatformState);
            Win32BeginInputPlayback(PlatformState, PlatformState->CurrentGameInputPlaybackBufferIndex);
            BOOL ReadResult = ReadFile
            (
                PlatformState->CurrentInputPlaybackFileHandle, GameInput,
                sizeof(game_input), &BytesRead, 0
            );
        }
    }
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

static void
Win32ProcessPendingMessagesSynchronous
(
    win32_platform_state *PlatformState,
    controller_state *KeyboardController
)
{
    MSG WindowsMessage;
    while (PeekMessage(&WindowsMessage, 0, 0, 0, PM_REMOVE))
    {
        switch (WindowsMessage.message)
        {
            case WM_QUIT:
            {
                PlatformState->IsRunning = FALSE;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VirtualKeyCode = (u32)WindowsMessage.wParam;
                b32 KeyWasDown = (WindowsMessage.lParam & (1 << 30)) != 0;
                b32 KeyIsDown = (WindowsMessage.lParam & (1ll << 31)) == 0;
                b32 AltKeyIsDown = (WindowsMessage.lParam & (1 << 29)) != 0;

                if (KeyIsDown != KeyWasDown)
                {
                    if (VirtualKeyCode == 'W')
                    {
                        Win32ProcessButton(&KeyboardController->MoveUp, KeyIsDown);
                    }
                    else if (VirtualKeyCode == 'A')
                    {
                        Win32ProcessButton(&KeyboardController->MoveLeft, KeyIsDown);
                    }
                    else if (VirtualKeyCode == 'S')
                    {
                        Win32ProcessButton(&KeyboardController->MoveDown, KeyIsDown);
                    }
                    else if (VirtualKeyCode == 'D')
                    {
                        Win32ProcessButton(&KeyboardController->MoveRight, KeyIsDown);
                    }
                    else if (VirtualKeyCode == 'Q')
                    {
                        Win32ProcessButton(&KeyboardController->LeftShoulder, KeyIsDown);
                    }
                    else if (VirtualKeyCode == 'E')
                    {
                        Win32ProcessButton(&KeyboardController->RightShoulder, KeyIsDown);
                    }
                    else if (VirtualKeyCode == VK_UP)
                    {
                        Win32ProcessButton(&KeyboardController->ActionUp, KeyIsDown);
                    }
                    else if (VirtualKeyCode == VK_DOWN)
                    {
                        Win32ProcessButton(&KeyboardController->ActionDown, KeyIsDown);
                    }
                    else if (VirtualKeyCode == VK_LEFT)
                    {
                        Win32ProcessButton(&KeyboardController->ActionLeft, KeyIsDown);
                    }
                    else if (VirtualKeyCode == VK_RIGHT)
                    {
                        Win32ProcessButton(&KeyboardController->ActionRight, KeyIsDown);
                    }
                    else if (VirtualKeyCode == VK_ESCAPE)
                    {
                        Win32ProcessButton(&KeyboardController->Back, KeyIsDown);
                        //PlatformState->IsRunning = FALSE;
                    }
                    else if (VirtualKeyCode == VK_SPACE)
                    {
                        Win32ProcessButton(&KeyboardController->Start, KeyIsDown);
                    }
#if HANDMADE_INTERNAL
                    else if (VirtualKeyCode == 'P' && KeyIsDown)
                    {
                        PlatformState->Pause = !PlatformState->Pause;
                    }
                    else if (VirtualKeyCode == 'L' && KeyIsDown)
                    {
                        if (PlatformState->CurrentGameInputPlaybackBufferIndex == 0)
                        {
                            if (PlatformState->CurrentGameInputRecordingBufferIndex == 0)
                            {
                                Win32BeginRecordingInput(PlatformState, 1);
                            }
                            else
                            {
                                Win32EndRecordingInput(PlatformState);
                                Win32BeginInputPlayback(PlatformState, 1);
                            }
                        }
                        else
                        {
                            Win32EndInputPlayback(PlatformState);
                        }
                    }
#endif // HANDMADE_INTERNAL
                    if ((VirtualKeyCode == VK_F4) && KeyIsDown && AltKeyIsDown)
                    {
                        PlatformState->IsRunning = FALSE;
                    }
                    else if ((VirtualKeyCode == VK_RETURN) && KeyIsDown && AltKeyIsDown)
                    {
                        if (WindowsMessage.hwnd)
                        {
                            Win32ToggleFullScreen(WindowsMessage.hwnd, &Win32PlatformState);
                        }
                    }
                }
            } break;

            default:
            {
                TranslateMessage(&WindowsMessage);
                DispatchMessage(&WindowsMessage);
            } break;
        }
    }
}

static LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

static f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, i64 CounterFrequency)
{
    f32 SecondsElapsed =
        (f32)(End.QuadPart - Start.QuadPart)
        / (f32)CounterFrequency;
    return SecondsElapsed;
}

int
WinMain
(
    HINSTANCE CurrentApplicationInstance,
    HINSTANCE PreviousApplicationInstance,
    LPSTR CommandLine,
    i32 ShowCmdOptions
)
{
    Win32PlatformState.PreviousWindowPlacement =
    {
        sizeof(WINDOWPLACEMENT)
    };

    Win32LoadXInput();

    LARGE_INTEGER PerformanceCounterFrequency;
    QueryPerformanceFrequency(&PerformanceCounterFrequency);

    Win32GetEXEFilePath(&Win32PlatformState);

    char GameCodeDLLPath[MAX_PATH];
    Win32BuildFilePath(&Win32PlatformState, (char *)"game.dll", sizeof(GameCodeDLLPath), GameCodeDLLPath);

    char TempDLLPath[MAX_PATH];
    Win32BuildFilePath(&Win32PlatformState, (char *)"game_temp.dll", sizeof(TempDLLPath), TempDLLPath);

    char CompilationLockFilePath[MAX_PATH];
    Win32BuildFilePath(&Win32PlatformState, (char *)"compilation.lock", sizeof(CompilationLockFilePath), CompilationLockFilePath);

    b32 SleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);

#if HANDMADE_INTERNAL
    Win32PlatformState.ShowCursor = TRUE;
#endif // HANDMADE_INTERNAL

    WNDCLASSA MainWindowClass = {};
    MainWindowClass.style = CS_VREDRAW | CS_HREDRAW;
    MainWindowClass.lpfnWndProc = Win32MainWindowCallback;
    MainWindowClass.hInstance = CurrentApplicationInstance;
    MainWindowClass.lpszClassName = "MainWindowClass";
    MainWindowClass.hCursor = LoadCursorA(0, IDC_ARROW);

    if (RegisterClassA(&MainWindowClass))
    {
        HWND MainWindow = CreateWindowExA
        (
            0, //WS_EX_TOPMOST | WS_EX_LAYERED,
            MainWindowClass.lpszClassName,
            "HandmadeHero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, CurrentApplicationInstance, 0
        );

        if (MainWindow)
        {
            i32 MonitorRefreshHz = 60;

            HDC MainWindowDeviceContextHandle = GetDC(MainWindow);
            i32 Win32RefreshRate = GetDeviceCaps(MainWindowDeviceContextHandle, VREFRESH);
            ReleaseDC(MainWindow, MainWindowDeviceContextHandle);

            if (Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            f32 GameRefreshHz = (MonitorRefreshHz / 2.0f);
            f32 TargetSecondsPerFrame = 1.0f / (f32)GameRefreshHz;

            Win32ResizePixelBuffer(&Win32PlatformState.BackBuffer, 960, 540);

            win32_sound_state SoundBufferState = {};

            win32_sound_configuration SoundBufferConfiguration = {};
            SoundBufferConfiguration.SamplesPerSecond = 48000;
            SoundBufferConfiguration.BytesPerSample = 2 * sizeof(i16);
            SoundBufferConfiguration.SoundBufferSize =
                1 * SoundBufferConfiguration.SamplesPerSecond * SoundBufferConfiguration.BytesPerSample;
            u32 SoundBytesProducedPerFrame = (u32)
            (
                (f32)(SoundBufferConfiguration.SamplesPerSecond * SoundBufferConfiguration.BytesPerSample) /
                GameRefreshHz
            );
            SoundBufferConfiguration.SafetyMarginInBytes = (u32)((f32)SoundBytesProducedPerFrame / 3.0f);

            Win32InitDSound(MainWindow, &SoundBufferConfiguration, &Win32PlatformState.SecondarySoundBuffer);
            Win32ClearSoundBuffer(&SoundBufferConfiguration, Win32PlatformState.SecondarySoundBuffer);

            Win32PlatformState.SecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

            i16 *GameSoundSamples = (i16 *)VirtualAlloc
            (
                0, SoundBufferConfiguration.SoundBufferSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
            );

#if HANDMADE_INTERNAL
            LPVOID AllocationBaseAddress = 0;
#else
            LPVOID AllocationBaseAddress = TeraBytes((u64)2);
#endif // HANDMADE_INTERNAL
            game_memory GameMemory = {};
            GameMemory.IsInitialized = FALSE;
            GameMemory.PlatformFreeFileMemory = PlatformFreeFileMemory;
            GameMemory.PlatformWriteFile = PlatformWriteFile;
            GameMemory.PlatformReadFile = PlatformReadFile;
            GameMemory.PermanentStorageSize = MegaBytes(256);
            GameMemory.TransientStorageSize = MegaBytes((u64)256); // casey has this at 1GB

            Win32PlatformState.GameMemoryBlockSize =
                GameMemory.TransientStorageSize + GameMemory.PermanentStorageSize;
            Win32PlatformState.GameMemoryBlock = VirtualAlloc
            (
                AllocationBaseAddress, (size_t)Win32PlatformState.GameMemoryBlockSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
            );

            GameMemory.PermanentStorage = Win32PlatformState.GameMemoryBlock;
            GameMemory.TransientStorage =
                (u8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

            for
            (
                i32 BufferIndex = 1;
                BufferIndex < ArrayCount(Win32PlatformState.GameInputBuffers);
                BufferIndex++
            )
            {
                win32_input_buffer *GameInputBuffer =
                    &Win32PlatformState.GameInputBuffers[BufferIndex];

                Win32GetInputOrStateFileLocation
                (
                    BufferIndex, FALSE, &Win32PlatformState,
                    sizeof(GameInputBuffer->InputBufferFileName),
                    GameInputBuffer->InputBufferFileName
                );

                GameInputBuffer->FileHandle = CreateFileA
                (
                    GameInputBuffer->InputBufferFileName,
                    GENERIC_READ | GENERIC_WRITE, 0, 0,
                    CREATE_ALWAYS, 0, 0
                );

                LARGE_INTEGER MaxSize;
                MaxSize.QuadPart = Win32PlatformState.GameMemoryBlockSize;

                GameInputBuffer->MemoryMapHandle = CreateFileMappingA
                (
                    GameInputBuffer->FileHandle, 0, PAGE_READWRITE,
                    MaxSize.HighPart, MaxSize.LowPart, 0
                );

                GameInputBuffer->MemoryBlock = MapViewOfFile
                (
                    GameInputBuffer->MemoryMapHandle, FILE_MAP_ALL_ACCESS,
                    0, 0, Win32PlatformState.GameMemoryBlockSize
                );
            }

            if (GameSoundSamples && Win32PlatformState.GameMemoryBlock)
            {
                u32 DebugTimeMarkerIndex = 0;
                win32_debug_sound_time_marker DebugTimeMarkers[30] = {0};
                b32 SoundIsValid = FALSE;

                game_input GameInputDoubleBuffer[2] = {};
                game_input *NewTotalGameInput = &GameInputDoubleBuffer[0];
                game_input *OldTotalGameInput = &GameInputDoubleBuffer[1];

                NewTotalGameInput->TimeDeltaForFrame = TargetSecondsPerFrame;
                OldTotalGameInput->TimeDeltaForFrame = TargetSecondsPerFrame;

                LARGE_INTEGER LastPerfCount = Win32GetWallClock();
                u64 LastCycleCount = __rdtsc();

                win32_game_code GameCode = Win32LoadGameCode(GameCodeDLLPath, TempDLLPath);

                thread_context ThreadContext = {};

                Win32PlatformState.IsRunning = TRUE;
                while (Win32PlatformState.IsRunning)
                {
                    FILETIME LastWriteTimeForDLL = Win32GetFileLastWriteTime(GameCodeDLLPath);
                    if (CompareFileTime(&LastWriteTimeForDLL, &GameCode.LastWriteTimeForLoadedDLL) != 0)
                    {
                        WIN32_FILE_ATTRIBUTE_DATA IgnoredParameter = {};
                        if
                        (
                            !GetFileAttributesExA
                            (
                                CompilationLockFilePath,
                                GetFileExInfoStandard,
                                &IgnoredParameter
                            )
                        )
                        {
                            Win32UnloadGameCode(&GameCode);
                            GameCode = Win32LoadGameCode(GameCodeDLLPath, TempDLLPath);
                        }
                    }

                    controller_state *OldKeyboardController = GetController(OldTotalGameInput, 0);
                    controller_state *NewKeyboardController = GetController(NewTotalGameInput, 0);
                    *NewKeyboardController = {};

                    DWORD MaxControllerCount = XUSER_MAX_COUNT;
                    if (MaxControllerCount > (ArrayCount(NewTotalGameInput->ControllerStates) - 1))
                    {
                        MaxControllerCount = (ArrayCount(NewTotalGameInput->ControllerStates) - 1);
                    }

                    for
                    (
                        int ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ButtonIndex++
                    )
                    {
                        NewKeyboardController->Buttons[ButtonIndex].IsDown =
                            OldKeyboardController->Buttons[ButtonIndex].IsDown;
                    }
                    NewKeyboardController->IsConnected = TRUE;

                    Win32ProcessPendingMessagesSynchronous(&Win32PlatformState, NewKeyboardController);

                    if (!Win32PlatformState.Pause)
                    {
                        POINT MousePosition;
                        GetCursorPos(&MousePosition);
                        ScreenToClient(MainWindow, &MousePosition);

                        NewTotalGameInput->MouseX = MousePosition.x;
                        NewTotalGameInput->MouseY = MousePosition.y;
                        NewTotalGameInput->MouseZ = 0;

                        Win32ProcessButton
                        (
                            &NewTotalGameInput->MouseButtons[0],
                            GetKeyState(VK_LBUTTON) & (1 << 15)
                        );

                        Win32ProcessButton
                        (
                            &NewTotalGameInput->MouseButtons[1],
                            GetKeyState(VK_MBUTTON) & (1 << 15)
                        );

                        Win32ProcessButton
                        (
                            &NewTotalGameInput->MouseButtons[2],
                            GetKeyState(VK_RBUTTON) & (1 << 15)
                        );

                        Win32ProcessButton
                        (
                            &NewTotalGameInput->MouseButtons[3],
                            GetKeyState(VK_XBUTTON1) & (1 << 15)
                        );

                        Win32ProcessButton
                        (
                            &NewTotalGameInput->MouseButtons[4],
                            GetKeyState(VK_XBUTTON2) & (1 << 15)
                        );

                        for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ControllerIndex++)
                        {
                            controller_state *OldControllerState =
                                GetController(OldTotalGameInput, ControllerIndex + 1);

                            controller_state *NewControllerState =
                                GetController(NewTotalGameInput, ControllerIndex + 1);

                            XINPUT_STATE SampledControllerState;
                            if (XInputGetState(ControllerIndex, &SampledControllerState) == ERROR_SUCCESS)
                            {
                                Win32CaptureGamepadControllerSample
                                (
                                    NewControllerState,
                                    OldControllerState,
                                    &SampledControllerState
                                );
                            }
                            else
                            {
                                NewControllerState->IsConnected = FALSE;
                            }
                        }

                        game_pixel_buffer LocalPixelsBuffer = {};
                        LocalPixelsBuffer.PixelsMemory = Win32PlatformState.BackBuffer.PixelsMemory;
                        LocalPixelsBuffer.WidthInPixels = Win32PlatformState.BackBuffer.WidthInPixels;
                        LocalPixelsBuffer.HeightInPixels = Win32PlatformState.BackBuffer.HeightInPixels;
                        LocalPixelsBuffer.BytesPerRow = Win32PlatformState.BackBuffer.BytesPerRow;
                        LocalPixelsBuffer.BytesPerPixel = Win32PlatformState.BackBuffer.BytesPerPixel;

                        if (Win32PlatformState.CurrentGameInputRecordingBufferIndex)
                        {
                            Win32RecordInput(&Win32PlatformState, NewTotalGameInput);
                        }

                        if (Win32PlatformState.CurrentGameInputPlaybackBufferIndex)
                        {
                            Win32PlaybackInput(&Win32PlatformState, NewTotalGameInput);
                        }

                        if (GameCode.UpdateAndRender)
                        {
                            GameCode.UpdateAndRender
                            (
                                &ThreadContext,
                                &LocalPixelsBuffer,
                                NewTotalGameInput,
                                &GameMemory
                            );
                        }

                        LARGE_INTEGER AudioOutputWallClock = Win32GetWallClock();
                        f32 FromFrameBeginningToAudioSeconds =
                            Win32GetSecondsElapsed
                            (
                                LastPerfCount,
                                AudioOutputWallClock,
                                PerformanceCounterFrequency.QuadPart
                            );

                        DWORD PlayCursor, WriteCursor;
                        if
                        (
                            SUCCEEDED
                            (
                                Win32PlatformState.SecondarySoundBuffer->
                                    GetCurrentPosition(&PlayCursor, &WriteCursor)
                            )
                        )
                        {
                            if (!SoundIsValid)
                            {
                                SoundBufferState.RunningSampleIndex =
                                    WriteCursor / SoundBufferConfiguration.BytesPerSample;
                                SoundIsValid = TRUE;
                            }

                            DWORD ByteToLock =
                                (SoundBufferState.RunningSampleIndex * SoundBufferConfiguration.BytesPerSample) %
                                SoundBufferConfiguration.SoundBufferSize;

                            f32 SecondsLeftUntilFlip = TargetSecondsPerFrame - FromFrameBeginningToAudioSeconds;

                            u32 ExpectedBytesUntilFlip = (u32)
                            (
                                (f32)SoundBytesProducedPerFrame *
                                SecondsLeftUntilFlip /
                                TargetSecondsPerFrame
                            );
                            u32 ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;

                            u32 SafeWriteCursor = WriteCursor;
                            if (SafeWriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundBufferConfiguration.SoundBufferSize;
                            }
                            Assert(SafeWriteCursor > PlayCursor);
                            SafeWriteCursor += SoundBufferConfiguration.SafetyMarginInBytes;

                            b32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

                            DWORD TargetCursor = 0;
                            if (AudioCardIsLowLatency)
                            {
                                TargetCursor = ExpectedFrameBoundaryByte + SoundBytesProducedPerFrame;
                            }
                            else
                            {
                                TargetCursor =
                                    WriteCursor +
                                    SoundBufferConfiguration.SafetyMarginInBytes +
                                    SoundBytesProducedPerFrame;
                            }
                            TargetCursor = TargetCursor % SoundBufferConfiguration.SoundBufferSize;

                            DWORD BytesToWrite = 0;
                            if (ByteToLock > TargetCursor)
                            {
                                BytesToWrite =
                                    (SoundBufferConfiguration.SoundBufferSize - ByteToLock) +
                                    TargetCursor;
                            }
                            else
                            {
                                BytesToWrite = TargetCursor - ByteToLock;
                            }

                            game_sound_request GameSoundRequest = {};
                            GameSoundRequest.SamplesPerSecond =
                                SoundBufferConfiguration.SamplesPerSecond;
                            GameSoundRequest.OutputSamplesCount =
                                BytesToWrite / SoundBufferConfiguration.BytesPerSample;
                            GameSoundRequest.OutputSamples = GameSoundSamples;

                            if (GameCode.GetSoundSamples)
                            {
                                GameCode.GetSoundSamples(&ThreadContext, &GameSoundRequest, &GameMemory);
                            }

                            Win32FillGlobalSoundBuffer
                            (
                                &SoundBufferConfiguration,
                                &SoundBufferState,
                                ByteToLock,
                                BytesToWrite,
                                Win32PlatformState.SecondarySoundBuffer,
                                &GameSoundRequest
                            );
#if HANDMADE_INTERNAL
                            win32_debug_sound_time_marker *TimeMarker =
                                &DebugTimeMarkers[DebugTimeMarkerIndex];
                            TimeMarker->OutputPlayCursor = PlayCursor;
                            TimeMarker->OutputWriteCursor = WriteCursor;
                            TimeMarker->OutputLocation = ByteToLock;
                            TimeMarker->OutputByteCount = BytesToWrite;
                            TimeMarker->ExpectedFlipCursor = ExpectedFrameBoundaryByte;

                            u32 UnwrappedWriteCursor = WriteCursor;
                            if (UnwrappedWriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundBufferConfiguration.SoundBufferSize;
                            }
                            u32 AudioLatencyInBytes = UnwrappedWriteCursor - PlayCursor;
                            f32 AudioLatencyInSeconds =
                                (f32)AudioLatencyInBytes /
                                (f32)SoundBufferConfiguration.BytesPerSample /
                                (f32)SoundBufferConfiguration.SamplesPerSecond;
#   if 0
                            char StringBuffer[512];
                            _snprintf_s(StringBuffer, sizeof(StringBuffer),
                                        "INFO:\n"
                                        "=================================================================\n"
                                        "PlayCursor:     %d   WriteCursor:  %d                            \n"
                                        "ByteToLock:     %d   BytesToWrite: %d   TargetCursor: %d         \n"
                                        "BytesBetweenCursors: %d                 AudioLatencyInSeconds: %f\n"
                                        "=================================================================\n",
                                        PlayCursor, WriteCursor, ByteToLock, BytesToWrite, TargetCursor,
                                        AudioLatencyInBytes, AudioLatencyInSeconds);
                            OutputDebugStringA(StringBuffer);
#   endif
#endif // HANDMADE_INTERNAL
                        }
                        else
                        {
                            SoundIsValid = FALSE;
                        }

                        f32 SecondsElapsedForWork = Win32GetSecondsElapsed
                        (
                            LastPerfCount,
                            Win32GetWallClock(),
                            PerformanceCounterFrequency.QuadPart
                        );
                        f32 SecondsElapsedForFrame = SecondsElapsedForWork;
                        if (SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {
                            if (SleepIsGranular)
                            {
                                DWORD SleepMilliSeconds = (DWORD)
                                    (1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                                if (SleepMilliSeconds > 0)
                                {
                                    Sleep(SleepMilliSeconds);
                                }
                            }

                            SecondsElapsedForFrame =
                                Win32GetSecondsElapsed
                                (
                                    LastPerfCount,
                                    Win32GetWallClock(),
                                    PerformanceCounterFrequency.QuadPart
                                );

                            if (SecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                do
                                {
                                    SecondsElapsedForFrame = Win32GetSecondsElapsed
                                    (
                                        LastPerfCount,
                                        Win32GetWallClock(),
                                        PerformanceCounterFrequency.QuadPart
                                    );
                                } while (SecondsElapsedForFrame < TargetSecondsPerFrame);
                            }
                            else
                            {
                                // NOTE: sleep() sleeped too much or missed frame
                            }
                        }
#if 0
                        f32 MilliSecondsPerFrame =
                            1000.0f *
                            Win32GetSecondsElapsed
                            (
                                LastPerfCount,
                                Win32GetWallClock(),
                                PerformanceCounterFrequency.QuadPart
                            );

                        f32 FramesPerSeconds = 1000.0f / MilliSecondsPerFrame;
                        u64 CPUCyclesElapsedSinceLastFrame = __rdtsc() - LastCycleCount;
                        char StringBuffer[256];
                        _snprintf_s
                        (
                            StringBuffer,
                            sizeof(StringBuffer),
                            "INFO: MilliSeconds Per Frame = %f | FPS = %f | MegaCycles Per Frame = %f\n",
                            MilliSecondsPerFrame, FramesPerSeconds, (f32)CPUCyclesElapsedSinceLastFrame / 1000000.0f
                        );
                        OutputDebugStringA(StringBuffer);
#endif
                        HDC DeviceContext = GetDC(MainWindow);
                        Win32DisplayBufferInWindow
                        (
                            DeviceContext,
                            Win32GetWindowDimensions(MainWindow),
                            &Win32PlatformState.BackBuffer
                        );
                        ReleaseDC(MainWindow, DeviceContext);

                        LastPerfCount = Win32GetWallClock();
                        LastCycleCount = __rdtsc();
#if HANDMADE_INTERNAL
                        DWORD FlipPlayCursor, FlipWriteCursor;
                        if
                        (
                            SUCCEEDED
                            (
                                Win32PlatformState.SecondarySoundBuffer->GetCurrentPosition
                                (
                                    &FlipPlayCursor,
                                    &FlipWriteCursor
                                )
                            )
                        )
                        {
                            Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
                            win32_debug_sound_time_marker *TimeMarker =
                                &DebugTimeMarkers[DebugTimeMarkerIndex];
                            TimeMarker->FlipPlayCursor = FlipPlayCursor;
                            TimeMarker->FlipWriteCursor = FlipWriteCursor;
                        }

                        DebugTimeMarkerIndex++;
                        if (DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers))
                        {
                            DebugTimeMarkerIndex = 0;
                        }
#endif // HANDMADE_INTERNAL

                        game_input *TempGameInput = NewTotalGameInput;
                        NewTotalGameInput = OldTotalGameInput;
                        OldTotalGameInput = TempGameInput;
                    }
                }
            }
        }
    }

    return (0);
}