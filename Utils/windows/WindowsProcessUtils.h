#pragma once
#include <Windows.h>

#include <vector>
#include <string>

namespace utils {
    namespace windows {
        namespace process {
            bool KillProcess(DWORD pid);
            bool WriteProcess(DWORD pid, const std::vector<byte>& buffer);
            HANDLE DuplicateProcessToken(DWORD pid);
            std::vector<DWORD> FindProcess(const std::wstring& name);
        };  // namespace process
    };  // namespace windows
};  // namespace utils
