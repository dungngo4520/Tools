#include "EncodingUtils.h"

#include <codecvt>

std::wstring utils::encoding::Utf8ToUnicode(const std::string& str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
}

std::string utils::encoding::UnicodeToUtf8(const std::wstring& str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str);
}
