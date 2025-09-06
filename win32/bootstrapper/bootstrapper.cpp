#include <Windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <direct.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"

#include "win32\build\actions\build_context.h"

// #include "win32\shared\system\memory.h"
// #include "win32\shared\strings\strings.h"
// #include "win32\shared\strings\string_list.h"
// #include "win32\shared\shell\console.h"
// #include "win32\shared\file_system\folders.h"
// #include "win32\shared\strings\path_handling.h"
// #include "win32\build\actions\msvc.h"
// #include "win32\build\targets.h"
// #include "win32\build\build.h"

b32 BuildBuild(build_context *BuildContext)
{
    b32 BuildSuccess = FALSE;

    char *StaticCompilerFlags =
        "/nologo /Oi /FC /GR- /EHa- "
        "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 "
        "/D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS ";

    char *DynamicCompilerFlags = NULL;
    if
    (
        (BuildContext->EnvironmentInfo.argc == 2) &&
        (strcmp(BuildContext->EnvironmentInfo.argv[1], "debug") == 0)
    )
    {
        DynamicCompilerFlags = "/Od /Z7 ";
    }
    else
    {
        DynamicCompilerFlags = "/O2 ";
    }

    char *SourceFiles[]
    {
        "\\win32\\shared\\strings\\path_handling.cpp",
        "\\win32\\shared\\strings\\string_list.cpp",
        "\\win32\\shared\\shell\\console.cpp",
        "\\win32\\shared\\file_system\\folders.cpp",
        "\\win32\\shared\\file_system\\files.cpp",
        "\\win32\\shared\\file_system\\fat12\\fat12_interface.cpp",
        "\\win32\\shared\\file_system\\fat12\\fat12_get.cpp",
        "\\win32\\shared\\file_system\\fat12\\fat12_set.cpp",
        "\\win32\\shared\\system\\processes.cpp",
        "\\win32\\tools\\build\\actions\\build_context.cpp",
        "\\win32\\tools\\build\\actions\\gcc.cpp",
        "\\win32\\tools\\build\\actions\\iverilog.cpp",
        "\\win32\\tools\\build\\actions\\msvc.cpp",
        "\\win32\\tools\\build\\actions\\nasm.cpp",
        "\\win32\\tools\\build\\targets\\hdl\\uart_app\\build.cpp",
        "\\win32\\tools\\build\\targets\\hdl\\verilog_demo\\build.cpp",
        "\\win32\\tools\\build\\targets\\i686-elf\\os\\bootloader\\build.cpp",
        "\\win32\\tools\\build\\targets\\i686-elf\\os\\bootsector\\build.cpp",
        "\\win32\\tools\\build\\targets\\i686-elf\\os\\kernel\\build.cpp",
        "\\win32\\tools\\build\\targets\\i686-elf\\os\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\build_tests\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\directx_demo\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\fat12_tests\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\fetch_data\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\handmade_hero\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\imgui_demo\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\lint\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\ray_tracer\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\refterm\\build.cpp",
        "\\win32\\tools\\build\\targets\\win32\\simulator\\build.cpp",
        "\\win32\\tools\\build\\build.cpp",
        "\\win32\\tools\\build\\targets.cpp",
    };

    for (u32 Index = 0; Index < ArrayCount(SourceFiles); Index++)
    {
        AddCompilerSourceFile(BuildContext, SourceFiles[Index]);

        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), "\\");
        char *SourceFileNameWithExtension = GetPointerToLastSegmentFromPath(SourceFiles[Index], '\\');
        char SourceFileName[_MAX_FNAME] = {};
        char SourceFileExtension[_MAX_EXT] = {};
        GetFileNameAndExtensionFromString(SourceFileNameWithExtension, SourceFileName, _MAX_FNAME, SourceFileExtension, _MAX_EXT);
        if (strcmp(SourceFileName, "build") == 0)
        {
            char TemporaryFilePathBuffer[MAX_PATH] = {};
            StringCbCatA(TemporaryFilePathBuffer, ArrayCount(TemporaryFilePathBuffer), SourceFiles[Index]);
            RemoveLastSegmentFromPath(TemporaryFilePathBuffer, FALSE, '\\');
            char *ParentFolderName = GetPointerToLastSegmentFromPath(TemporaryFilePathBuffer, '\\');
            StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), ParentFolderName);
            StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), "_");
            StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), SourceFileName);
        }
        else
        {
            StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), SourceFileName);
        }
        StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), ".obj");
        SetCompilerOutputObject(BuildContext, ObjectFileName);

        AddCompilerFlags(BuildContext, StaticCompilerFlags);
        AddCompilerFlags(BuildContext, DynamicCompilerFlags);
        SetCompilerIncludePath(BuildContext, "\\");
        BuildSuccess = CompileWithMSVC(BuildContext);
        if (!BuildSuccess)
        {
            return FALSE;
        }
        ClearBuildContext(BuildContext);
    }

    string_node *ObjectFiles = GetListOfFilesWithNameInFolder
    (
        BuildContext->EnvironmentInfo.TargetOutputDirectoryPath,
        "*.obj"
    );

    string_node *CurrentFileNode = ObjectFiles;
    while (CurrentFileNode)
    {
        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), "\\");
        StringCchCatA(ObjectFileName, ArrayCount(ObjectFileName), GetPointerToLastSegmentFromPath(CurrentFileNode->String, '\\'));
        AddLinkerInputFile(BuildContext, ObjectFileName);
        CurrentFileNode = CurrentFileNode->NextNode;
    }
    FreeStringList(ObjectFiles);

    AddLinkerFlags(BuildContext, "/subsystem:console /incremental:no /opt:ref user32.lib shell32.lib Shlwapi.lib");
    SetLinkerOutputBinary(BuildContext, "\\build.exe");
    BuildSuccess = LinkWithMSVC(BuildContext);
    return BuildSuccess;
}

int main(int argc, char **argv)
{
    u32 BuildSuccess = FALSE;
    build_context BuildContext = {};

    _getcwd
    (
        BuildContext.EnvironmentInfo.WorkspaceDirectoryPath,
        sizeof(BuildContext.EnvironmentInfo.WorkspaceDirectoryPath)
    );

    StringCchCatA
    (
        BuildContext.EnvironmentInfo.OutputsDirectoryPath,
        ArrayCount(BuildContext.EnvironmentInfo.OutputsDirectoryPath),
        BuildContext.EnvironmentInfo.WorkspaceDirectoryPath
    );
    StringCchCatA
    (
        BuildContext.EnvironmentInfo.OutputsDirectoryPath,
        ArrayCount(BuildContext.EnvironmentInfo.OutputsDirectoryPath),
        "\\outputs"
    );
    
    StringCchCatA
    (
        BuildContext.EnvironmentInfo.TargetOutputDirectoryPath,
        ArrayCount(BuildContext.EnvironmentInfo.TargetOutputDirectoryPath),
        BuildContext.EnvironmentInfo.OutputsDirectoryPath
    );
    StringCchCatA
    (
        BuildContext.EnvironmentInfo.TargetOutputDirectoryPath,
        ArrayCount(BuildContext.EnvironmentInfo.TargetOutputDirectoryPath),
        "\\win32\\tools\\build"
    );

    BuildContext.EnvironmentInfo.argc = argc;
    BuildContext.EnvironmentInfo.argv = argv;

    InitializeConsole();

    CreateDirectoryA(BuildContext.EnvironmentInfo.TargetOutputDirectoryPath, NULL);
    b32 Result = SetCurrentDirectoryA(BuildContext.EnvironmentInfo.TargetOutputDirectoryPath);
    if (Result)
    {
        BuildSuccess = BuildBuild(&BuildContext);
        SetCurrentDirectoryA(BuildContext.EnvironmentInfo.WorkspaceDirectoryPath);
    }

    if (BuildSuccess)
    {
        ConsolePrintColored("INFO: Build Succeeded.\n", FOREGROUND_GREEN);
        return 0;
    }
    else
    {
        ConsolePrintColored("ERROR: Build Failed.\n", FOREGROUND_RED);
        return 1;
    }
}