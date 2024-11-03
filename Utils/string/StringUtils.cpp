#include "StringUtils.h"

std::vector<unsigned char> utils::string::StringToByteArray(const std::string& str)
{
    return std::vector<unsigned char>(str.begin(), str.end());
}

#pragma warning(push, 1)
#pragma warning(disable : 4244)  // conversion from 'wchar_t' to 'unsigned char', possible loss of data
std::vector<unsigned char> utils::string::StringToByteArray(const std::wstring& str)
{
    return std::vector<unsigned char>(str.begin(), str.end());
}
#pragma warning(pop)
