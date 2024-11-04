#define UMDF_USING_NTSTATUS
#include <Windows.h>
#include <ntstatus.h>
#include <iostream>
#include <memory>

using namespace std;

#pragma comment(lib, "ntdll.lib")

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

enum FILE_INFORMATION_CLASS {
    FileNameInformation = 9,
    FileNormalizedNameInformation = 48,
};

EXTERN_C
NTSTATUS WINAPI NtQueryInformationFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
);

typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;


int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        wcout << L"Usage: " << argv[0] << L" <file path>" << endl;
        cout << "get long file name" << endl;
        return 0;
    }

    HANDLE hFile = CreateFileW(
        argv[1], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Failed to open file. Error " << GetLastError() << endl;
        return GetLastError();
    }

    cout << "open file ok" << endl;

    auto buffersize = 1024;
    unique_ptr<byte[]> buffer = unique_ptr<byte[]>(new byte[1024]);
    ZeroMemory(buffer.get(), buffersize);

    NTSTATUS status;
    IO_STATUS_BLOCK iosb = {};

    while ((status = NtQueryInformationFile(hFile, &iosb, buffer.get(), buffersize, FileNormalizedNameInformation)) ==
           STATUS_BUFFER_OVERFLOW) {
        if (status == STATUS_SUCCESS) break;
        PFILE_NAME_INFORMATION info = (PFILE_NAME_INFORMATION)buffer.get();
        buffersize = sizeof(FILE_NAME_INFORMATION) + info->FileNameLength;
        buffer.reset(new byte[buffersize]);
        ZeroMemory(buffer.get(), buffersize);
        cout << "Byte need " << buffersize << endl;
    }

    cout << "get file name done" << endl;

    if (status == STATUS_SUCCESS) {
        PFILE_NAME_INFORMATION info = (PFILE_NAME_INFORMATION)buffer.get();
        printf("File name %.*ws", info->FileNameLength / sizeof(wchar_t), info->FileName);
    }
    else {
        cout << "Failed to get file name. Error " << hex << status << dec << endl;
        return status;
    }

    CloseHandle(hFile);
    return 0;
}
