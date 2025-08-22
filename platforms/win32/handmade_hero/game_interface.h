#pragma once

#define PLATFORM_READ_FILE(name)\
    read_file_result name(thread_context *Thread, char *FileName)
#define PLATFORM_WRITE_FILE(name)\
    b32 name(thread_context *Thread, char *FileName, void *DataToWrite, u32 DataSize)
#define PLATFORM_FREE_FILE_MEMORY(name)\
    void name(thread_context *Thread, void *FileMemory)
#define GAME_UPDATE_AND_RENDER(name)\
    void name(thread_context *ThreadContext, game_pixel_buffer *PixelBuffer, game_input *GameInput, game_memory *GameMemory)
#define GAME_GET_SOUND_SAMPLES(name)\
    void name(thread_context *ThreadContext, game_sound_request *SoundRequest, game_memory *GameMemory)

struct read_file_result;
struct thread_context;
struct game_pixel_buffer;
struct game_input;
struct game_memory;
struct game_sound_request;

typedef PLATFORM_READ_FILE(platform_read_file);
typedef PLATFORM_WRITE_FILE(platform_write_file);
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

struct thread_context
{
    i32 PlaceHolder;
};

struct game_pixel_buffer
{
    void *PixelsMemory;
    i32 WidthInPixels;
    i32 HeightInPixels;
    i32 BytesPerPixel;
    i32 BytesPerRow;
};

struct game_sound_request
{
    u32 OutputSamplesCount;
    u32 SamplesPerSecond;
    i16 *OutputSamples;
};

struct button_state
{
    i32 TransitionsCount;
    b32 IsDown;
};

struct controller_state
{
    b32 IsConnected;
    b32 MovementIsAnalog;
    f32 StickAverageX;
    f32 StickAverageY;

    union
    {
        button_state Buttons[12];
        struct
        {
            button_state MoveUp;
            button_state MoveDown;
            button_state MoveLeft;
            button_state MoveRight;
            button_state ActionUp;
            button_state ActionDown;
            button_state ActionLeft;
            button_state ActionRight;
            button_state LeftShoulder;
            button_state RightShoulder;
            button_state Back;
            button_state Start;
            // NOTE: put all new buttons above the Start button
        };
    };
};

struct game_input
{
    f32 TimeDeltaForFrame;
    button_state MouseButtons[5];
    i32 MouseX;
    i32 MouseY;
    i32 MouseZ;
    controller_state ControllerStates[5];
};

struct game_memory
{
    b32 IsInitialized;

    u64 PermanentStorageSize;
    void *PermanentStorage;

    u64 TransientStorageSize;
    void *TransientStorage;

    platform_read_file *PlatformReadFile;
    platform_write_file *PlatformWriteFile;
    platform_free_file_memory *PlatformFreeFileMemory;
};

struct read_file_result
{
    void *FileContents;
    u32 ConstentsSize;
};

inline controller_state *
GetController(game_input *GameInput, i32 ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(GameInput->ControllerStates));
    return &GameInput->ControllerStates[ControllerIndex];
}