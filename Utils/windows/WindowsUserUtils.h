#pragma once
#include <Windows.h>

#include <string>

namespace utils {
    namespace windows {
        namespace user {
            bool IsElevated();
            bool IsPrivilegeEnabled(const std::wstring& privilegeName);
            bool SetPrivilege(const std::wstring& privilegeName, bool enable);
        };  // namespace user
    };  // namespace windows
};  // namespace utils
