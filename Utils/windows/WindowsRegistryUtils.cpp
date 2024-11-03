#include "WindowsRegistryUtils.h"

#include <stdio.h>

#pragma comment(lib, "Advapi32.lib")

bool utils::windows::registry::RenameKey(HKEY rootKey, const std::wstring& source, const std::wstring& target)
{
    auto status = RegRenameKey(rootKey, source.c_str(), target.c_str());
    if (status != ERROR_SUCCESS) {
        printf(__FUNCTION__ ": Failed to rename key. Error: %d\n", status);
        return false;
    }
    return true;
}
