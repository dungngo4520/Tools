#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

using namespace std;

std::string base64_encode(const std::string& in)
{
    std::string out;

    int val = 0, valb = -6;
    for (auto& c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]
        );
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

std::string base64_decode(const std::string& in)
{
    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (auto& c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string UnicodeToUtf8(LPCWSTR pszUnicode)
{
    LPSTR pszUtf8;
    int len, maxLen;
    if (pszUnicode == NULL) return "";
    len = WideCharToMultiByte(CP_UTF8, 0, pszUnicode, -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    maxLen = len;

    pszUtf8 = new char[len + 1];
    if (pszUtf8 == NULL) return "";

    len = WideCharToMultiByte(CP_UTF8, 0, pszUnicode, -1, pszUtf8, len, NULL, NULL);
    if (len > maxLen) {
        delete[] pszUtf8;
        return "";
    }
    pszUtf8[len] = 0;
    std::string result = pszUtf8;
    delete[] pszUtf8;
    return result;
}

std::string UnicodeToUtf8(const std::wstring& sUnicode) { return UnicodeToUtf8(sUnicode.c_str()); }

int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        wcout << L"Usage: " << argv[0] << L" <string>" << endl;
        wcout << L"cal base64 hash from <string>" << endl;
        return 0;
    }

    wcout << argv[1] << endl;

    string utf = UnicodeToUtf8(argv[1]);
    cout << "UTF8: " << utf << endl;

    string base64 = base64_encode(utf);
    cout << "Base64: " << base64 << endl;

    return 0;
}
