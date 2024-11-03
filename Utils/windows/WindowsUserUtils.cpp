#include "WindowsUserUtils.h"

#include <stdio.h>

#include <memory>

bool utils::windows::user::IsElevated()
{
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE) {
        printf(__FUNCTION__ ": Failed to open process token. Error: %d\n", GetLastError());
        return false;
    }

    TOKEN_ELEVATION Elevation = {};
    DWORD cbSize = sizeof(TOKEN_ELEVATION);
    if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize) == FALSE) {
        printf(__FUNCTION__ ": Failed to get token information. Error: %d\n", GetLastError());
    }
    CloseHandle(hToken);
    return Elevation.TokenIsElevated;
}

bool utils::windows::user::IsPrivilegeEnabled(const std::wstring& privilegeName)
{
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE) {
        printf(__FUNCTION__ ": Failed to open process token. Error: %d\n", GetLastError());
        return false;
    }

    LUID luid;
    if (LookupPrivilegeValueW(NULL, privilegeName.c_str(), &luid) == FALSE) {
        printf(__FUNCTION__ ": Failed to lookup privilege value. Error: %d\n", GetLastError());
        return false;
    }

    DWORD sizeNeeded = 0;
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &sizeNeeded);
    printf(__FUNCTION__ ": Size needed: %d\n", sizeNeeded);
    if (sizeNeeded == 0) return false;

    std::unique_ptr<byte[]> privilegesBuffer(new byte[sizeNeeded]);
    if (GetTokenInformation(hToken, TokenPrivileges, privilegesBuffer.get(), sizeNeeded, &sizeNeeded) == FALSE) {
        printf(__FUNCTION__ ": Failed to get token information. Error: %d\n", GetLastError());
        return false;
    }

    auto privileges = reinterpret_cast<TOKEN_PRIVILEGES*>(privilegesBuffer.get());
    for (unsigned int i = 0; i < privileges->PrivilegeCount; i++) {
        if (luid.LowPart == privileges->Privileges[i].Luid.LowPart &&
            luid.HighPart == privileges->Privileges[i].Luid.HighPart) {
            return privileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED;
        }
    }
    return false;
}

bool utils::windows::user::SetPrivilege(const std::wstring& privilegeName, bool enable)
{
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken) == FALSE) {
        printf(__FUNCTION__ ": Failed to open process token. Error: %d\n", GetLastError());
        return false;
    }

    TOKEN_PRIVILEGES tokenPrivileges;
    LUID luid;
    if (LookupPrivilegeValueW(NULL, privilegeName.c_str(), &luid) == FALSE) {
        printf(__FUNCTION__ ": Failed to lookup privilege value. Error: %d\n", GetLastError());
        return false;
    }

    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luid;
    tokenPrivileges.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
    if (AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof(tokenPrivileges), NULL, NULL) == FALSE) {
        printf(__FUNCTION__ ": Failed to adjust token privileges. Error: %d\n", GetLastError());
    }

    return GetLastError() == ERROR_SUCCESS;
}
