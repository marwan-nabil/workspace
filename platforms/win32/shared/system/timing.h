#pragma once

inline LARGE_INTEGER
GetWindowsTimerValue()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline f32
GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, i64 ProfileCounterFrequency)
{
    f32 SecondsElapsed =
        (f32)(End.QuadPart - Start.QuadPart) /
        (f32)ProfileCounterFrequency;
    return SecondsElapsed;
}

inline i64
GetWindowsTimerFrequency()
{
    LARGE_INTEGER ProfileCounterFrequency;
    QueryPerformanceFrequency(&ProfileCounterFrequency);
    return ProfileCounterFrequency.QuadPart;
}