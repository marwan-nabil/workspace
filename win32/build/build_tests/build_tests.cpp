// TODO:
// - improve compilation tests console messages:
// - info messages should denote the beginning and ending of the test clearly,
//   because all the tests are usually run sequentially and output in a single log.
// - it should be indicated clearly if not all tests have succeeded.
// - failed tests have extra error information provided.
// - build dependency graph of #include'ed files in compile targets.
// - store the test results permanently, with info about dependencies.
// - check on the last modification before running a test for a target.
// - only rerun the test if necessary, meaning, if one of the target
//   dependencies changed since the last test run.
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <strsafe.h>
#include <io.h>

#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\shell\console.h"
#include "sources\win32\libraries\system\timing.h"
#include "sources\win32\libraries\system\processes.h"
#include "sources\win32\tests\build_tests\build_tests.h"

test_job_configuration TestJobConfiguration[] =
{
    {NULL, "build simulator"},
    {"imgui_demo", "build imgui_demo opengl2"},
    {"imgui_demo", "build imgui_demo dx11"},
    {"ray_tracer", "build ray_tracer 1_lane"},
    {"ray_tracer", "build ray_tracer 4_lanes"},
    {"ray_tracer", "build ray_tracer 8_lanes"},
    {NULL, "build handmade_hero"},
    {"directx_demo", "build directx_demo debug"},
    {"directx_demo", "build directx_demo release"},
    {NULL, "build lint"},
    {NULL, "build fetch_data"},
    {NULL, "build fat12_tests"},
    {NULL, "build os"},
    {NULL, "build verilog_demo"},
    {NULL, "build uart_app"},
    {NULL, "build refterm"},
};

void ProcessJob(test_job *Job)
{
    SECURITY_ATTRIBUTES ChildProcessOutputPipeSecturityAttributes = {};
    ChildProcessOutputPipeSecturityAttributes.bInheritHandle = TRUE;

    HANDLE PipeOutput;
    HANDLE PipeInput;
    CreatePipe(&PipeOutput, &PipeInput, &ChildProcessOutputPipeSecturityAttributes, 0);
    SetHandleInformation(PipeOutput, HANDLE_FLAG_INHERIT, 0);

    HANDLE MutexHandle = 0;
    if (Job->MutexName)
    {
        MutexHandle = CreateMutexA(NULL, FALSE, Job->MutexName);
        WaitForSingleObject(MutexHandle, INFINITE);
    }

    Job->TestResult = CreateProcessAndWait(Job->TestCommand, PipeInput);
    CloseHandle(PipeInput);

    if (Job->MutexName)
    {
        ReleaseMutex(MutexHandle);
        CloseHandle(MutexHandle);
    }

    char PipeOutputBuffer[1024] = {};
    DWORD BytesRead;
    ReadFile(PipeOutput, PipeOutputBuffer, 1024, &BytesRead, NULL);
    StringCchCat(Job->TestOutputBuffer, Job->TestOutputBufferSize, PipeOutputBuffer);
}

DWORD WINAPI
WorkerThreadEntry(void *Parameter)
{
    test_job_queue *JobQueue = (test_job_queue *)Parameter;

    while (JobQueue->TotalJobsDone < JobQueue->JobCount)
    {
        u64 JobIndex = InterlockedExchangeAdd64(&JobQueue->NextJobIndex, 1);
        if (JobIndex >= JobQueue->JobCount)
        {
            return TRUE;
        }

        ProcessJob(&JobQueue->Jobs[JobIndex]);
        InterlockedExchangeAdd64(&JobQueue->TotalJobsDone, 1);
    }

    return TRUE;
}

i32 main(i32 argc, char **argv)
{
    i64 WindowsTimerFrequency = GetWindowsTimerFrequency();
    LARGE_INTEGER StartTime = GetWindowsTimerValue();

    test_job_queue JobQueue;
    JobQueue.JobCount = ArrayCount(TestJobConfiguration);
    JobQueue.NextJobIndex = 0;
    JobQueue.TotalJobsDone = 0;
    JobQueue.Jobs = (test_job *)malloc(JobQueue.JobCount * sizeof(test_job));

    for (u32 JobIndex = 0; JobIndex < JobQueue.JobCount; JobIndex++)
    {
        test_job *Job = &JobQueue.Jobs[JobIndex];
        ZeroMemory(Job, sizeof(test_job));

        Job->MutexName = TestJobConfiguration[JobIndex].MutexName;
        Job->TestCommand = TestJobConfiguration[JobIndex].TestCommand;
        Job->TestResult = FALSE;
        Job->TestOutputBufferSize = 1024;
        Job->TestOutputBuffer = (char *)malloc(Job->TestOutputBufferSize);
        ZeroMemory(Job->TestOutputBuffer, Job->TestOutputBufferSize);
    }

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    u32 ThreadCount = (u8)SystemInfo.dwNumberOfProcessors;

    MemoryBarrier();

    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
    {
        DWORD ThreadId;
        HANDLE ThreadHandle = CreateThread(0, 0, WorkerThreadEntry, &JobQueue, 0, &ThreadId);
        CloseHandle(ThreadHandle);
    }

    while (JobQueue.TotalJobsDone < JobQueue.JobCount) {}

    b32 AllTestsSucceeded = TRUE;

    InitializeConsole();

    for (u32 JobIndex = 0; JobIndex < JobQueue.JobCount; JobIndex++)
    {
        test_job *Job = &JobQueue.Jobs[JobIndex];

        printf("\n");
        fflush(stdout);
        ConsoleSwitchColor(BACKGROUND_BLUE);
        printf("> %s", Job->TestCommand);
        fflush(stdout);
        ConsoleResetColor();
        printf("\n");
        fflush(stdout);

        printf("%s", Job->TestOutputBuffer);

        if (Job->TestResult)
        {
            ConsolePrintColored("INFO: test succeeded.\n", FOREGROUND_GREEN);
        }
        else
        {
            ConsolePrintColored("ERROR: test failed.\n", FOREGROUND_RED);
            AllTestsSucceeded = FALSE;
        }
    }

    if (AllTestsSucceeded)
    {
        ConsolePrintColored
        (
            "\n==========================\n"
            "INFO: all tests succeeded.\n"
            "==========================\n",
            FOREGROUND_GREEN
        );
    }

    for (u32 JobIndex = 0; JobIndex < JobQueue.JobCount; JobIndex++)
    {
        free(JobQueue.Jobs[JobIndex].TestOutputBuffer);
    }
    free(JobQueue.Jobs);

    f32 SecondsElapsed = GetSecondsElapsed(StartTime, GetWindowsTimerValue(), WindowsTimerFrequency);
    printf("INFO: time elapsed for all tests: %f seconds\n", SecondsElapsed);

    return 0;
}