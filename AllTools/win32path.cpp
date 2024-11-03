#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>

using namespace std;

#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

wstring GetWin32Path(const wstring& NativeFileName)
{
    NTSTATUS status = 0;
    HANDLE hFile = NULL;
    OBJECT_ATTRIBUTES oa = {};
    UNICODE_STRING FileName = {};
    IO_STATUS_BLOCK iosb = {};
    unique_ptr<wchar_t> Buffer;
    DWORD BufferSize = 0;
    bool success = false;

    const static wchar_t ExtPrefix[] = L"\\\\?\\";
    const static size_t ExtPrefixLength = wcslen(ExtPrefix);
    const static wchar_t UNCPrefix[] = L"UNC";
    const static size_t UNCPrefixLength = wcslen(UNCPrefix);
    const static wchar_t MUPPrefix[] = L"\\Device\\Mup";
    const static size_t MUPPrefixLength = wcslen(MUPPrefix);

    FileName.Length = FileName.MaximumLength = (USHORT)((NativeFileName.length()) * sizeof(wchar_t));
    FileName.Buffer = (wchar_t*)NativeFileName.c_str();

    InitializeObjectAttributes(&oa, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hFile,
        FILE_READ_ATTRIBUTES,
        &oa,
        &iosb,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        0,
        NULL,
        0
    );
    if (status != 0) {
        cout << __FUNCTION__ ": Failed to open file " << hex << status << endl;
        goto Exit;
    }

    BufferSize = GetFinalPathNameByHandleW(hFile, Buffer.get(), BufferSize, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (BufferSize == 0) goto Exit;

    Buffer.reset(new wchar_t[BufferSize]);
    if (!Buffer) goto Exit;

    BufferSize = GetFinalPathNameByHandleW(hFile, Buffer.get(), BufferSize, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (BufferSize == 0) goto Exit;

    success = true;

Exit:
    if (hFile) CloseHandle(hFile);
    if (success) {
        wstring path = Buffer.get();
        if (_wcsnicmp(path.c_str(), ExtPrefix, ExtPrefixLength) == 0) {
            path = path.substr(ExtPrefixLength);
        }

        // check network path
        if (_wcsnicmp(path.c_str(), UNCPrefix, UNCPrefixLength) == 0) {
            path = L"\\" + path.substr(UNCPrefixLength);
        }

        return path;
    }
    else {
        // Failed to parse network path
        if (_wcsnicmp(NativeFileName.c_str(), MUPPrefix, MUPPrefixLength) == 0) {
            return L"\\" + NativeFileName.substr(MUPPrefixLength);
        }

        // Fallback
        return NativeFileName;
    }
}

bool GetKernelPath(const wstring& win32Path, wstring& kernelPath)
{
    HANDLE hFile = NULL;
    OBJECT_ATTRIBUTES oa = {};
    UNICODE_STRING FileName = {};
    IO_STATUS_BLOCK iosb = {};
    unique_ptr<wchar_t> Buffer;
    DWORD BufferSize = 0;

    hFile = CreateFileW(
        win32Path.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = CreateFileW(
            win32Path.c_str(),
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_DIRECTORY | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );
        if (hFile == INVALID_HANDLE_VALUE) {
            cout << "Failed to open file " << GetLastError() << endl;
            return false;
        }
    }

    BufferSize = GetFinalPathNameByHandleW(hFile, Buffer.get(), BufferSize, FILE_NAME_NORMALIZED | VOLUME_NAME_NT);
    if (BufferSize == 0) {
        cout << "Failed to get file path " << hex << GetLastError() << endl;
        CloseHandle(hFile);
        return false;
    }

    Buffer.reset(new wchar_t[BufferSize]);
    if (!Buffer) {
        cout << "Failed to alloc mem" << endl;
        CloseHandle(hFile);
        return false;
    }

    BufferSize = GetFinalPathNameByHandleW(hFile, Buffer.get(), BufferSize, FILE_NAME_NORMALIZED | VOLUME_NAME_NT);
    if (BufferSize == 0) {
        cout << "Failed to get file path " << hex << GetLastError() << endl;
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    kernelPath = Buffer.get();
    return true;
}

bool GetKernelPath1(const wstring& win32Path, wstring& kernelPath, bool& needResolve)
{
    wstring path = win32Path;
    wstring temp;
    size_t pos = 0;

    UNREFERENCED_PARAMETER(needResolve);

    // check network path
    if (path.substr(0, 2) == L"\\\\") {
        kernelPath = L"UNC" + win32Path.substr(1);  // prefix with UNC and let the kernel resolve path
        return true;
    }

    if (GetKernelPath(win32Path, temp)) {
        kernelPath = temp;
        return true;
    }

    pos = path.find_last_of(L'\\');
    while (pos != wstring::npos && pos > 1) {
        if (GetKernelPath(path.substr(0, pos + 1), temp)) {
            kernelPath = temp + win32Path.substr(pos);
            return true;
        }

        if (GetKernelPath(path.substr(0, pos), temp)) {
            kernelPath = temp + win32Path.substr(pos);
            return true;
        }
        path = path.substr(0, pos);
        pos = path.find_last_of(L'\\');
    }

    return false;
};

void printhelp(const wstring& name)
{
    wcout << L"Usage: " << name << L" w2k|k2w <path>" << endl;
    wcout << L"w2k: Resolve win32 mode <path> to kernel mode path" << endl;
    wcout << L"k2w: Resolve kernel mode <path> to user mode path" << endl;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3) {
        printhelp(argv[0]);
    }

    wstring op = argv[1];

    if (op == L"w2k") {
        wstring kernelPath;
        bool needresolve = false;
        if (GetKernelPath1(argv[2], kernelPath, needresolve)) {
            wcout << kernelPath << endl;
            if (needresolve) {
                cout << "Path still need resolve again" << endl;
            }
        }
        else {
            cout << "Failed to resolve path" << endl;
        }
    }
    else if (op == L"k2w") {
        wcout << GetWin32Path(argv[2]) << endl;
    }
    else {
        printhelp(argv[0]);
    }

    return 0;
}
