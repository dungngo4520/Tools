#include "WindowsProcessUtils.h"

#include <stdio.h>

bool utils::windows::process::KillProcess(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        printf(__FUNCTION__ ": Failed to open process. Error: %d\n", GetLastError());
        return false;
    }
    if (TerminateProcess(hProcess, 0) == FALSE) {
        printf(__FUNCTION__ ": Failed to terminate process. Error: %d\n", GetLastError());
        CloseHandle(hProcess);
        return false;
    }
    CloseHandle(hProcess);
    return true;
}

bool utils::windows::process::WriteProcess(DWORD pid, const std::vector<byte>& buffer)
{
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf(__FUNCTION__ ": Failed to open process. Error: %d\n", GetLastError());
        return false;
    }

    auto StartAddress = VirtualAllocEx(hProcess, NULL, buffer.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (StartAddress == NULL) {
        printf(__FUNCTION__ ": Failed to allocate memory. Error: %d\n", GetLastError());
        CloseHandle(hProcess);
        return false;
    }

    if (WriteProcessMemory(hProcess, StartAddress, buffer.data(), buffer.size(), NULL) == FALSE) {
        printf(__FUNCTION__ ": Failed to write process memory. Error: %d\n", GetLastError());
        CloseHandle(hProcess);
        return false;
    }
    CloseHandle(hProcess);
    return true;
}
