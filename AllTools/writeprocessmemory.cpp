#include "../Utils/windows/WindowsUtils.h"
#include "../Utils/string/StringUtils.h"

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3) {
        printf("Usage: writeprocessmemory <pid> <data...>\n");
        return 0;
    }

    DWORD pid = _wtoi(argv[1]);

    std::wstring data;
    for (int i = 2; i < argc; i++) {
        data += argv[i];
        data += L" ";
    }

    printf("Writing to process %d\n", pid);

    if (!utils::windows::user::IsElevated()) {
        printf("Run again with Administrator.");
        return 1;
    }

    if (utils::windows::process::WriteProcess(pid, utils::string::StringToByteArray(data))) {
        printf("Success\n");
    }
    else {
        printf("Failed\n");
    }
    return 0;
}
