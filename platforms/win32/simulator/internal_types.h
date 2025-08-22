#pragma once

struct window_private_data
{
    rendering_buffer *LocalRenderingBuffer;
    b32 *RunningState;
};

struct button_state
{
    i32 TransitionCount;
    b32 IsDown;
};

struct user_input
{
    union
    {
        struct
        {
            button_state Up;
            button_state Down;
            button_state Left;
            button_state Right;
            button_state Shift;
        };
        button_state Buttons[5];
    };

    b8 ExitSignal;
};