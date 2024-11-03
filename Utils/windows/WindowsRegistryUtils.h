#pragma once
#include <Windows.h>

#include <string>

namespace utils {
    namespace windows {
        namespace registry {

            void EnumerateKeyCallback(HKEY rootKey, const std::wstring& keyPath);

            bool RenameKey(HKEY rootKey, const std::wstring& source, const std::wstring& target);
            bool EnumerateKey(HKEY rootKey, const std::wstring& keyPath);
        };  // namespace registry

    };  // namespace windows
};  // namespace utils
