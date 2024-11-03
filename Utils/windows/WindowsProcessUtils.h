#pragma once
#include <Windows.h>

#include <vector>

namespace utils {
    namespace windows {
        namespace process {
            bool KillProcess(DWORD pid);
            bool WriteProcess(DWORD pid, const std::vector<byte>& buffer);
        };  // namespace process
    };  // namespace windows
};  // namespace utils
