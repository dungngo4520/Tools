#define CURL_STATICLIB
#include "../Library/libcurl/include/curl/curl.h"

#include "../Utils/encoding/EncodingUtils.h"

#include <string>
#include <codecvt>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Normaliz.lib")

#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "../../../Library/libcurl/lib/windows_x64/libcurl_a_debug.lib")
#else
#pragma comment(lib, "../../../Library/libcurl/lib/windows_x64/libcurl_a.lib")
#endif  // _DEBUG
#else
#ifdef _DEBUG
#pragma comment(lib, "../../../Library/libcurl/lib/windows_x86/libcurl_a_debug.lib")
#pragma
#else
#pragma comment(lib, "../../../Library/libcurl/lib/windows_x86/libcurl_a.lib")
#endif  // _DEBUG
#endif  // _WIN64

static size_t curl_write_cb(void* data, size_t size, size_t nmemb, void* userp)
{
    std::wstring* response = (std::wstring*)userp;
    int realsize = size * nmemb;
    *response = *response + utils::encoding::Utf8ToUnicode(std::string((char*)data, realsize));
    return realsize;
}

template <typename... Args>
static std::string StringFormat(const char* format, Args... args)
{
    auto size = static_cast<size_t>(std::snprintf(nullptr, 0, format, args...)) + 1;  // include null
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format, args...);
    buf.get()[size - 1] = '\0';
    return buf.get();
}
template <typename... Args>
static std::wstring StringFormat(const wchar_t* format, Args... args)
{
    auto size = static_cast<size_t>(std::swprintf(nullptr, 0, format, args...)) + 1;  // include null
    std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
    std::swprintf(buf.get(), size, format, args...);
    buf.get()[size - 1] = '\0';
    return buf.get();
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 6) {
        printf("%ls <count> <url> <service> <method> <payload>\n", argv[0]);
        return 1;
    }
    int i = 1;
    int count = _wtoi(argv[i++]);
    std::wstring url = argv[i++];
    std::wstring service = argv[i++];
    std::wstring method = argv[i++];
    std::wstring payload = argv[i++];

    printf(
        "Calling curl on %ls, service '%ls' with method '%ls', payload: '%ls' %d time%s\n",
        url.c_str(),
        service.c_str(),
        method.c_str(),
        payload.c_str(),
        count,
        count > 1 ? "s" : ""
    );

    auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        printf("Failed to call curl_global_init. Error: %d\n", code);
        return 1;
    }

    for (i = 1; i <= count; ++i) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            printf("[%d] Failed to call curl_easy_init\n", i);
            return 1;
        }
        char errbuf[CURL_ERROR_SIZE] = {};

        std::wstring response = L"";

        code = curl_easy_setopt(curl, CURLOPT_URL, utils::encoding::UnicodeToUtf8(argv[2]).c_str());
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_PROXY, "");
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/rpcx");
        headers = curl_slist_append(headers, "X-RPCX-MesssageType: 0");
        headers = curl_slist_append(headers, "X-RPCX-SerializeType: 1");
        headers = curl_slist_append(headers, StringFormat("X-RPCX-ServicePath: %ls", service.c_str()).c_str());
        headers = curl_slist_append(headers, StringFormat("X-RPCX-ServiceMethod: %ls", method.c_str()).c_str());

        code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }


        code = curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, utils::encoding::UnicodeToUtf8(payload).c_str());
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        code = curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, NULL);
        if (code != CURLE_OK) {
            printf("[%d] Line %d: Failed to call curl_easy_setopt. Code: %d\n", i, __LINE__, code);
            return 1;
        }

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        if (CURLE_OK != res) {
            printf("failed after %d attempts\nlibcurl: (%d) %s\n", i, res, errbuf);
            return 2;
        }

        printf("[%d] get %ls -> %ls\n", i, argv[2], response.c_str());
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
