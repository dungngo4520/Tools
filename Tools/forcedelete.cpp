#include "../Utils/windows/WindowsUtils.h"
#include "../Utils/string/StringUtils.h"
#include "../Utils/memory/MemoryUtils.h"

#include "../Utils/cli/CLI11.hpp"

#include <Windows.h>
#include <TlHelp32.h>

#include <stdio.h>

int wmain(int argc, const wchar_t** argv)
{
    CLI::App app("Simple file delete tool.");

    std::wstring filename;
    app.add_option("--file", filename, "Target file to delete")->required();

    CLI11_PARSE(app, argc, argv);

    printf("Deleting: %ls\n", filename.c_str());

    if (DeleteFileW(argv[1]) == TRUE) {
        printf("Delete file %ls success", argv[1]);
        return 0;
    }
    else {
        printf(
            "Delete file failed: (%d)%ls",
            GetLastError(),
            utils::windows::error::TranslateSystemError(GetLastError()).c_str()
        );
    }

    if (argc == 3) {
        if (utils::windows::user::IsElevated()) {
            HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == INVALID_HANDLE_VALUE) {
                printf(
                    "Failed to query processes: (%d)%ls",
                    GetLastError(),
                    utils::windows::error::TranslateSystemError(GetLastError()).c_str()
                );
                return 1;
            }
            defer { CloseHandle(hProcessSnap); };

            PROCESSENTRY32W pe32 = {};
            pe32.dwSize = sizeof(PROCESSENTRY32W);
            if (!Process32FirstW(hProcessSnap, &pe32)) {
                printf(
                    "Failed to iterate process: (%d)%ls",
                    GetLastError(),
                    utils::windows::error::TranslateSystemError(GetLastError()).c_str()
                );
                return 1;
            }

            do {
                auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                if (hProcess == NULL) {
                    // maybe critical process
                    continue;
                }


            } while (Process32NextW(hProcessSnap, &pe32));
        }
        else {
            printf("Process does not has administrator right. Elevating...");
            // todo
        }
    }


    return 1;
}
