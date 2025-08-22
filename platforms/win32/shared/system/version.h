#pragma once

typedef u32 (WINAPI *rtl_verify_version_info)(OSVERSIONINFOEXW *, u32, ULONGLONG);

b32 IsWindowsVersionGreaterOrEqual
(
    HMODULE NtDllModule,
    u16 MajorVersion,
    u16 MinorVersion,
    u16 PatchVersion
);