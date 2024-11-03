#include <string>

namespace utils {
    namespace encoding {
        std::wstring Utf8ToUnicode(const std::string& str);
        std::string UnicodeToUtf8(const std::wstring& str);
    };  // namespace encoding

};  // namespace utils
