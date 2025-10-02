#include <Windows.h>
#include <winhttp.h>
#include <stdint.h>
#include <stdio.h>
#include <io.h>
#include <strsafe.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\strings.h"

struct data_chunk
{
    void *ChunkMemory;
    u32 ChunkSize;
    data_chunk *NextChunk;
};

data_chunk *GetRequestChunks(HINTERNET RequestHandle, u32 *TotalDownloadSize)
{
    data_chunk *FirstChunk = 0;
    data_chunk *PreviousChunk = 0;
    data_chunk *CurrentChunk = 0;

    while (1)
    {
        DWORD AvailableDataSize = 0;
        WinHttpQueryDataAvailable(RequestHandle, &AvailableDataSize);

        if (AvailableDataSize == 0)
        {
            break;
        }

        CurrentChunk = (data_chunk *)malloc(sizeof(data_chunk));
        *CurrentChunk = {};

        CurrentChunk->ChunkMemory = malloc(AvailableDataSize);
        ZeroMemory(CurrentChunk->ChunkMemory, AvailableDataSize);
        CurrentChunk->ChunkSize = AvailableDataSize;

        DWORD DownloadedDataSize = 0;
        WinHttpReadData(RequestHandle, CurrentChunk->ChunkMemory, AvailableDataSize, &DownloadedDataSize);
        *TotalDownloadSize += DownloadedDataSize;

        if (PreviousChunk)
        {
            PreviousChunk->NextChunk = CurrentChunk;
        }
        else
        {
            FirstChunk = CurrentChunk;
        }
        PreviousChunk = CurrentChunk;
    }

    return FirstChunk;
}

i32 main(i32 argc, char **argv)
{
    HINTERNET SessionHandle = WinHttpOpen
    (
        L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    HINTERNET ConnectionHandle = WinHttpConnect
    (
        SessionHandle,
        L"www.microsoft.com",
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );

    HINTERNET RequestHandle = WinHttpOpenRequest
    (
        ConnectionHandle,
        L"GET",
        NULL,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );

    wchar_t *HeadersString =
        L"Referer: https://www.google.com/\n"
        L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36\n";

    b32 Result = WinHttpSendRequest
    (
        RequestHandle,
        HeadersString,
        (DWORD)wcslen(HeadersString),
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    );

    Result = WinHttpReceiveResponse(RequestHandle, NULL);

    u32 TotalDownloadSize = 0;
    data_chunk *CurrentChunk = GetRequestChunks(RequestHandle, &TotalDownloadSize);

    HANDLE FileHandle = CreateFileA
    (
        "read_file.html", FILE_APPEND_DATA, 0, 0,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );

    while (CurrentChunk)
    {
        DWORD BytesWritten;
        WriteFile(FileHandle, CurrentChunk->ChunkMemory, CurrentChunk->ChunkSize, &BytesWritten, 0);

        free(CurrentChunk->ChunkMemory);
        CurrentChunk = CurrentChunk->NextChunk;
    }

    CloseHandle(FileHandle);
    return 0;
}