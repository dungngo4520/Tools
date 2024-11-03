#pragma once
#include <Windows.h>

#include <string>

namespace utils {
    namespace windows {
        namespace error {
            std::wstring TranslateSystemError(DWORD error);
        };
    };  // namespace windows
};  // namespace utils
