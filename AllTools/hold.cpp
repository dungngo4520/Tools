#include <Windows.h>
#include <iostream>

using namespace std;

int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        wcout << L"Usage: " << argv[0] << L" <file path>" << endl;
        wcout << L"Hold file, release at exit" << endl;
        return 0;
    }

    HANDLE hFile = CreateFileW(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wcout << L"Failed to open file " << argv[1] << L". Error " << GetLastError() << endl;
        return GetLastError();
    }

    wcout << L"Held file " << argv[1] << endl;

    system("pause");
    CloseHandle(hFile);

    wcout << L"Release file " << argv[1] << endl;

    return 0;
}