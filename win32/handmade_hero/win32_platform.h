#pragma once

#include <windows.h>
#include <dsound.h>
#include "win32\shared\base_types.h"

#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
#define DIRECTSOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

typedef XINPUT_GET_STATE(xinput_get_state);
typedef XINPUT_SET_STATE(xinput_set_state);
typedef DIRECTSOUND_CREATE(directsound_create);

struct win32_pixel_buffer
{
    BITMAPINFO BitmapInfo;
    void *PixelsMemory;
    i32 WidthInPixels;
    i32 HeightInPixels;
    i32 BytesPerPixel;
    i32 BytesPerRow;
};

struct win32_window_dimensions
{
    i32 WidthInPixels;
    i32 HeightInPixels;
};

struct win32_sound_configuration
{
    u32 SamplesPerSecond;
    u32 BytesPerSample;
    u32 SoundBufferSize;
    u32 SafetyMarginInBytes;
};

struct win32_sound_state
{
    u32 RunningSampleIndex;
};

struct win32_debug_sound_time_marker
{
    u32 OutputPlayCursor;
    u32 OutputWriteCursor;
    u32 OutputLocation;
    u32 OutputByteCount;
    u32 FlipPlayCursor;
    u32 FlipWriteCursor;
    u32 ExpectedFlipCursor;
};

struct win32_game_code
{
    HMODULE GameCodeDll;
    FILETIME LastWriteTimeForLoadedDLL;
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;
    b32 IsValid;
};

struct win32_input_buffer
{
    char InputBufferFileName[MAX_PATH];
    HANDLE FileHandle;
    HANDLE MemoryMapHandle;
    void *MemoryBlock;
};

struct win32_platform_state
{
    u64 GameMemoryBlockSize;
    void *GameMemoryBlock;

    win32_input_buffer GameInputBuffers[4];

    u32 CurrentGameInputRecordingBufferIndex;
    HANDLE CurrentGameInputRecordingFileHandle;

    u32 CurrentGameInputPlaybackBufferIndex;
    HANDLE CurrentInputPlaybackFileHandle;

    char ExeFilePath[MAX_PATH];
    char *ExeFilePathOnePastLastSlash;

    b32 IsRunning;
    b32 Pause;
    b32 ShowCursor;

    win32_pixel_buffer BackBuffer;
    LPDIRECTSOUNDBUFFER SecondarySoundBuffer;
    WINDOWPLACEMENT PreviousWindowPlacement;
};

XINPUT_GET_STATE(XInputGetStateStub);
XINPUT_SET_STATE(XInputSetStateStub);