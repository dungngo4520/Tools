#include "../Utils/windows/WindowsUtils.h"
#include <Windows.h>

#include <stdio.h>

#include <string>


int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        printf("Usage: runassystem <command...>\n");
        return 0;
    }

    std::wstring command;
    for (int i = 1; i < argc; i++) {
        command += argv[i];
        command += L" ";
    }

    printf("Executing command as system: %ls\n", command.c_str());

    if (utils::windows::user::IsElevated()) {
        printf("Run again with Administrator.");
        return 1;
    }

    if (utils::windows::user::SetPrivilege(SE_DEBUG_NAME, true)) {
        printf("Failed to gain privilege\n");
        return 1;
    }

    return 0;
}
