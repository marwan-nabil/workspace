#include <Windows.h>
#include <stdint.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "version.h"

b32 IsWindowsVersionGreaterOrEqual
(
    HMODULE NtDllModule,
    u16 MajorVersion,
    u16 MinorVersion,
    u16 PatchVersion
)
{
    rtl_verify_version_info RtlVerifyVersionInfoFunction = NULL;

    if (NtDllModule)
    {
        RtlVerifyVersionInfoFunction = (rtl_verify_version_info)GetProcAddress
        (
            NtDllModule,
            "RtlVerifyVersionInfo"
        );
    }

    if (RtlVerifyVersionInfoFunction == NULL)
    {
        return FALSE;
    }

    RTL_OSVERSIONINFOEXW VersionInfo = {};
    VersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    VersionInfo.dwMajorVersion = MajorVersion;
    VersionInfo.dwMinorVersion = MinorVersion;

    u64 ConditionMask = 0;

    VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    u32 Result = RtlVerifyVersionInfoFunction(&VersionInfo, VER_MAJORVERSION | VER_MINORVERSION, ConditionMask);
    if (Result == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}