#include <Windows.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <codecvt>
#include <locale>
static std::string UnicodeToUtf8(const std::wstring& str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str);
}
static std::string TranslateWinError(DWORD Error)
{
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL
    );
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3) {
        std::cout << "changetime <file> <c|a|m> <time>" << std::endl;
        std::cout << "Change file time." << std::endl;
        std::cout << "\tfile\tfilepath" << std::endl;
        std::cout << "\tc\tCreation time" << std::endl;
        std::cout << "\ta\tAccess time" << std::endl;
        std::cout << "\tm\tModity time" << std::endl;
        std::cout << "\ttime\tformatted as yyyy/mm/dd-hh:MM:ss" << std::endl;
        return 0;
    }

    int i = 1;
    std::wstring file = argv[i++];
    std::wstring option = argv[i++];
    std::wstring timestring = argv[i++];
    std::istringstream ss(UnicodeToUtf8(timestring));
    ss.imbue(std::locale("de_DE.utf-8"));

    std::tm tm = {};
    ss >> std::get_time(&tm, "%Y/%m/%d-%H:%M:%S");
    if (ss.fail()) {
        std::cout << "Failed to parse time" << std::endl;
        return 1;
    }

    HANDLE hFile = CreateFileW(
        file.c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "Open file error: " << TranslateWinError(GetLastError()) << std::endl;
        return 1;
    }

    SYSTEMTIME st = {};
    FILETIME ft = {};

    st.wYear = (WORD)tm.tm_year + 1900;
    st.wMonth = (WORD)tm.tm_mon + 1;
    st.wDay = (WORD)tm.tm_mday;
    st.wHour = (WORD)tm.tm_hour;
    st.wMinute = (WORD)tm.tm_min;
    st.wSecond = (WORD)tm.tm_sec;
    SystemTimeToFileTime(&st, &ft);

    auto setCreate = option.find(L"c") != option.npos;
    auto setAccess = option.find(L"a") != option.npos;
    auto setModify = option.find(L"m") != option.npos;

    if (SetFileTime(hFile, setCreate ? &ft : NULL, setAccess ? &ft : NULL, setModify ? &ft : NULL) == FALSE) {
        std::cout << "Set file time error: " << TranslateWinError(GetLastError()) << std::endl;
        return 1;
    }
    std::cout << "Set file time ok" << std::endl;
    return 0;
}
