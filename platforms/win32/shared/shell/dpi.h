#pragma once

typedef DPI_AWARENESS_CONTEXT (WINAPI *set_thread_dpi_awareness)(DPI_AWARENESS_CONTEXT);
typedef HRESULT (WINAPI *set_process_dpi_awareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT (WINAPI *get_dpi_for_monitror)(HMONITOR, MONITOR_DPI_TYPE, u32 *, u32 *);

b32 ConfigureDpiAwareness(HMODULE NtDllModule, HMODULE User32DllModule);
f32 GetDpiScaleForMonitor(HMODULE NtDllModule, HMODULE ShcoreDllModule, void *Monitor);