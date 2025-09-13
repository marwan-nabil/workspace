#pragma once

#include "win32\shared\base_types.h"

struct directory_node
{
    char DirectoryRelativePath[1024];
    directory_node *NextDirectoryNode;
};

struct file_node
{
    char FileRelativePath[1024];
    file_node *NextFileNode;
};

enum lint_job_type
{
    LJT_FILE_LIST,
    LJT_DIRECTORY,
    LJT_FILE
};

struct lint_job
{
    lint_job_type Type;

    union
    {
        struct
        {
            u32 NumberOfFiles;
            char **FileRelativePaths;
        };
        char *DirectoryRelativePath;
        char *FileRelativePath;
    };

    b32 Result;
};

struct lint_job_queue
{
    u32 JobCount;
    lint_job *Jobs;
    volatile i64 NextJobIndex;
    volatile i64 TotalJobsDone;
};