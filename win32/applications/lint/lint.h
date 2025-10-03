#pragma once

#include "portable\shared\base_types.h"

struct lint_job
{
    char *FilePath;
    b32 Result;
};

struct lint_job_queue
{
    u32 JobCount;
    lint_job *Jobs;
    volatile i64 NextJobIndex;
    volatile i64 TotalJobsDone;
};