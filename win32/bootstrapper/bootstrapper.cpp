#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include <stdio.h>
#include <direct.h>

#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\strings\strings.h"
#include "sources\win32\libraries\strings\string_list.h"
#include "sources\win32\libraries\shell\console.h"
#include "sources\win32\libraries\file_system\folders.h"
#include "sources\win32\libraries\strings\path_handling.h"
#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\actions\msvc.h"
#include "sources\win32\tools\build\targets.h"
#include "sources\win32\tools\build\build.h"

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
        "\\sources\\win32\\libraries\\strings\\path_handling.cpp",
        "\\sources\\win32\\libraries\\strings\\string_list.cpp",
        "\\sources\\win32\\libraries\\shell\\console.cpp",
        "\\sources\\win32\\libraries\\file_system\\folders.cpp",
        "\\sources\\win32\\libraries\\file_system\\files.cpp",
        "\\sources\\win32\\libraries\\file_system\\fat12\\fat12_interface.cpp",
        "\\sources\\win32\\libraries\\file_system\\fat12\\fat12_get.cpp",
        "\\sources\\win32\\libraries\\file_system\\fat12\\fat12_set.cpp",
        "\\sources\\win32\\libraries\\system\\processes.cpp",
        "\\sources\\win32\\tools\\build\\actions\\build_context.cpp",
        "\\sources\\win32\\tools\\build\\actions\\gcc.cpp",
        "\\sources\\win32\\tools\\build\\actions\\iverilog.cpp",
        "\\sources\\win32\\tools\\build\\actions\\msvc.cpp",
        "\\sources\\win32\\tools\\build\\actions\\nasm.cpp",
        "\\sources\\win32\\tools\\build\\targets\\hdl\\uart_app\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\hdl\\verilog_demo\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\i686-elf\\os\\bootloader\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\i686-elf\\os\\bootsector\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\i686-elf\\os\\kernel\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\i686-elf\\os\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\build_tests\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\directx_demo\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\fat12_tests\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\fetch_data\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\handmade_hero\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\imgui_demo\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\lint\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\ray_tracer\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\refterm\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets\\win32\\simulator\\build.cpp",
        "\\sources\\win32\\tools\\build\\build.cpp",
        "\\sources\\win32\\tools\\build\\targets.cpp",
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