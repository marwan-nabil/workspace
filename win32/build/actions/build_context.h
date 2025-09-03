#pragma once

struct environment_info
{
    i32 argc;
    char **argv;
    char WorkspaceDirectoryPath[1024];
    char OutputsDirectoryPath[1024];
    char TargetOutputDirectoryPath[1024];
};

struct compilation_info
{
    char CompilerFlags[1024];
    char CompilerIncludePath[1024];
    string_node *Sources;
    char OutputObjectPath[1024];
};

struct linking_info
{
    char LinkerFlags[1024];
    char LinkerScriptPath[1024];
    string_node *LinkerInputs;
    char OutputBinaryPath[1024];
};

struct build_context
{
    environment_info EnvironmentInfo;
    compilation_info CompilationInfo;
    linking_info LinkingInfo;
};

void AddCompilerFlags(build_context *BuildContext, const char *Flags);
void SetCompilerIncludePath(build_context *BuildContext, const char *IncludePath);
void AddCompilerSourceFile(build_context *BuildContext, const char *SourceFile);
void SetCompilerOutputObject(build_context *BuildContext, const char *ObjectFile);
void AddLinkerFlags(build_context *BuildContext, const char *Flags);
void SetLinkerScriptPath(build_context *BuildContext, const char *LinkerScriptFile);
void AddLinkerInputFile(build_context *BuildContext, char *LinkerInputFile);
void SetLinkerOutputBinary(build_context *BuildContext, char *OutputBinaryPath);
void PushSubTarget(build_context *BuildContext, const char *SubTargetRelativePath);
void PopSubTarget(build_context *BuildContext);
void ClearBuildContext(build_context *BuildContext);