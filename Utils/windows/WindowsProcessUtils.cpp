#include "WindowsProcessUtils.h"

#include <TlHelp32.h>

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

HANDLE utils::windows::process::DuplicateProcessToken(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf(__FUNCTION__ ": Failed to open process. Error: %d\n", GetLastError());
        return NULL;
    }
    HANDLE hToken = NULL;
    if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken) == FALSE) {
        printf(__FUNCTION__ ": Failed to open process token. Error: %d\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }

    HANDLE hDupToken = NULL;
    if (DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hDupToken) == FALSE) {
        printf(__FUNCTION__ ": Failed to duplicate token. Error: %d\n", GetLastError());
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return NULL;
    }
    CloseHandle(hToken);
    CloseHandle(hProcess);
    return hDupToken;
}

std::vector<DWORD> utils::windows::process::FindProcess(const std::wstring& name)
{
    std::vector<DWORD> result;
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry)) {
        do {
            if (name == std::wstring(entry.szExeFile)) {
                result.push_back(entry.th32ProcessID);
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return result;
}
