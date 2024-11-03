#pragma once
#include <stdio.h>

#include <string>
#include <memory>
#include <vector>

namespace utils {
    namespace string {
        template <typename... Args>
        static std::string StringFormat(const char* format, Args... args)
        {
            auto size = static_cast<size_t>(std::snprintf(nullptr, 0, format, args...)) + 1;
            std::unique_ptr<char[]> buf(new char[size]);
            std::snprintf(buf.get(), size, format, args...);
            buf.get()[size - 1] = '\0';
            return buf.get();
        }
        template <typename... Args>
        static std::wstring StringFormat(const wchar_t* format, Args... args)
        {
            auto size = static_cast<size_t>(std::swprintf(nullptr, 0, format, args...)) + 1;
            std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
            std::swprintf(buf.get(), size, format, args...);
            buf.get()[size - 1] = '\0';
            return buf.get();
        }

        std::vector<unsigned char> StringToByteArray(const std::string& str);
        std::vector<unsigned char> StringToByteArray(const std::wstring& str);
    };  // namespace string
};      // namespace utils
