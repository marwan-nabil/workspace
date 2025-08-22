#pragma once

struct test_job
{
    char *TestOutputBuffer;
    u32 TestOutputBufferSize;
    char *MutexName;
    char *TestCommand;
    b32 TestResult;
};

struct test_job_configuration
{
    char *MutexName;
    char *TestCommand;
};

struct test_job_queue
{
    u32 JobCount;
    test_job *Jobs;
    volatile i64 NextJobIndex;
    volatile i64 TotalJobsDone;
};