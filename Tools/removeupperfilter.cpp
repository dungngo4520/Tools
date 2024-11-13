#include <Windows.h>

#include <time.h>

#include <string>
#include <list>
#include <vector>
#include <iterator>
#include <set>
#include <memory>

void buildStringListFromVector(std::list<std::wstring>& filtersList, std::vector<WCHAR>& valVector)
{
    std::wstring currFilter;
    std::wstring::size_type currPos = 0;

    do {
        currFilter = &valVector[currPos];

        if (!currFilter.empty()) {
            filtersList.push_back(currFilter);
        }

        currPos += currFilter.size() + 1;
        if (currPos >= valVector.size()) {
            break;
        }

    } while (!currFilter.empty());
}

void buildNewListWithoutElement(
    std::list<std::wstring>& newfiltersList,
    std::list<std::wstring>& filtersList,
    std::wstring element
)
{
    for (auto filter : filtersList) {
        if (filter != element) {
            newfiltersList.push_back(filter);
        }
    }
}

void buildMultiStringVectorFromList(std::vector<WCHAR>& valVector, std::list<std::wstring>& newfiltersList)
{
    for (auto filter : newfiltersList) {
        std::copy(filter.begin(), filter.end(), std::back_inserter(valVector));
        valVector.push_back(TEXT('\0'));
    }
    if (valVector.empty()) {
        valVector.push_back(TEXT('\0'));
    }
    valVector.push_back(TEXT('\0'));
}

template <typename Func>
LSTATUS EnumRegistryKey(
    HKEY RootKey,
    const std::wstring& KeyPath,
    const DWORD& Flags,
    const std::set<std::wstring>& Filters,
    Func func
)
{
    HKEY hKey = NULL;
    LSTATUS status = ERROR_SUCCESS;
    DWORD Index = 0;

    DWORD NameBufferSize = 255;  // Max name length
    std::unique_ptr<wchar_t> NameBuffer = std::unique_ptr<wchar_t>(new wchar_t[NameBufferSize]);

    if (!NameBuffer.get()) return ERROR_INSUFFICIENT_BUFFER;

    status = RegOpenKeyExW(RootKey, KeyPath.c_str(), 0, KEY_READ | Flags, &hKey);
    if (status != ERROR_SUCCESS) return status;

    auto TempNameBufferSize = NameBufferSize;
    while (status == ERROR_SUCCESS) {
        status = RegEnumKeyExW(hKey, Index, NameBuffer.get(), &TempNameBufferSize, NULL, NULL, NULL, NULL);
        if (status != ERROR_SUCCESS) break;

        std::wstring SubKeyName = NameBuffer.get();
        bool callback = Filters.empty();

        for (auto& filter : Filters) {
            if (SubKeyName.find(filter) != std::wstring::npos) {
                callback = true;
                break;
            }
        }

        if (callback) func(KeyPath + L"\\" + SubKeyName);

        // Reset
        TempNameBufferSize = NameBufferSize;
        Index++;
    }

    RegCloseKey(hKey);

    return ERROR_SUCCESS;
}

template <typename Func>
LSTATUS EnumRegistryValue(
    HKEY RootKey,
    const std::wstring& KeyPath,
    const DWORD& Flags,
    const std::vector<DWORD>& TypeFilters,
    const std::vector<std::wstring>& NameFilters,
    Func func
)
{
    HKEY hKey = NULL;
    LSTATUS status = ERROR_SUCCESS;
    DWORD Index = 0;

    DWORD NameBufferSize = 32767;  // Max name length
    std::unique_ptr<wchar_t> Buffer = std::unique_ptr<wchar_t>(new wchar_t[NameBufferSize]);
    DWORD Type = 0;
    DWORD ValueBufferSize = 4096;
    std::unique_ptr<byte> ValueBuffer = std::unique_ptr<byte>(new byte[ValueBufferSize]);

    if (!Buffer.get() || !ValueBuffer.get()) return ERROR_INSUFFICIENT_BUFFER;

    RtlZeroMemory(Buffer.get(), NameBufferSize * sizeof(wchar_t));

    status = RegOpenKeyExW(RootKey, KeyPath.c_str(), 0, KEY_READ | Flags, &hKey);
    if (status != ERROR_SUCCESS) return status;

    DWORD TempNameBufferSize = NameBufferSize;
    DWORD TempValueBufferSize = ValueBufferSize;
    while (status == ERROR_SUCCESS) {
        status = RegEnumValueW(
            hKey, Index, Buffer.get(), &TempNameBufferSize, NULL, &Type, ValueBuffer.get(), &TempValueBufferSize
        );
        if (status == ERROR_MORE_DATA) {
            ValueBufferSize = TempValueBufferSize;
            ValueBuffer.reset(new byte[ValueBufferSize]);
            if (!ValueBuffer.get()) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            status = RegEnumValueW(
                hKey, Index, Buffer.get(), &TempNameBufferSize, NULL, &Type, ValueBuffer.get(), &TempValueBufferSize
            );
        }
        if (status != ERROR_SUCCESS) break;

        std::wstring ValueName = Buffer.get();
        bool matchType = TypeFilters.empty();
        bool matchName = NameFilters.empty();
        int TypeIndex = 0, NameIndex = 0;

        for (auto& type : TypeFilters) {
            if (type == Type) {
                matchType = true;
                break;
            }
            TypeIndex++;
        }

        for (auto& filter : NameFilters) {
            if (ValueName.find(filter) != std::wstring::npos) {
                matchName = true;
                break;
            }
            NameIndex++;
        }

        if (matchType && matchName) {
            func(TypeIndex, NameIndex, ValueName, ValueBuffer.get(), TempValueBufferSize);
        }

        // Reset
        TempNameBufferSize = NameBufferSize;
        TempValueBufferSize = ValueBufferSize;
        Index++;
    }

    RegCloseKey(hKey);

    return ERROR_SUCCESS;
}

bool EndsWith(const std::wstring& a, const std::wstring& b)
{
    if (b.size() > a.size()) return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

std::wstring g_BackupFile;
BOOL GetBackupFile()
{
    WCHAR fullpath[MAX_PATH] = {};

    srand((unsigned int)time(NULL));
    std::wstring name = L"%SystemDrive%\\" + std::to_wstring(rand()) + L"backup_class.reg";

    if (ExpandEnvironmentStringsW(name.c_str(), fullpath, MAX_PATH) == 0) {
        printf("Failed expand string %ls: %d\n", name.c_str(), GetLastError());
        return FALSE;
    }

    g_BackupFile = fullpath;
    printf("Backup file: %ls\n", g_BackupFile.c_str());

    return TRUE;
}

BOOL BackupClassKeys()
{
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};
    WCHAR fullpath[MAX_PATH] = {};

    if (ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\reg.exe", fullpath, MAX_PATH) == 0) {
        printf("Failed expand string: %d\n", GetLastError());
        return FALSE;
    }

    std::wstring processName = fullpath;
    printf("Reg path: %ls\n", processName.c_str());

    std::wstring cmd =
        processName + L" EXPORT HKLM\\SYSTEM\\ControlSet001\\Control\\Class \"" + g_BackupFile + L"\" /y";

    auto ret = CreateProcessW(processName.c_str(), (LPWSTR)cmd.c_str(), NULL, 0, FALSE, 0, NULL, NULL, &si, &pi);

    if (ret == FALSE) {
        printf("Failed to execute %d\n", GetLastError());
        return FALSE;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD ExitCode = 0;
    GetExitCodeProcess(pi.hProcess, &ExitCode);
    if (ExitCode != 0) {
        printf("Failed to execute %d, exitcode %d\n", GetLastError(), ExitCode);
        return FALSE;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}

void RestoreClassKeys()
{
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};

    WCHAR fullpath[MAX_PATH] = {};
    std::wstring processName;

    if (ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\reg.exe", fullpath, MAX_PATH) == 0) {
        printf("Failed expand string: %d\n", GetLastError());
        processName = L"C:\\Windows\\System32\\reg.exe";
    }
    else {
        processName = fullpath;
    }

    printf("Reg path: %ls\n", processName.c_str());

    std::wstring cmd = processName + L" IMPORT \"" + g_BackupFile + L"\"";

    auto ret = CreateProcessW(processName.c_str(), (LPWSTR)cmd.c_str(), NULL, 0, FALSE, 0, NULL, NULL, &si, &pi);

    if (ret == FALSE) {
        printf("Failed to execute %d\n", GetLastError());
        return;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD ExitCode = 0;
    GetExitCodeProcess(pi.hProcess, &ExitCode);

    if (ExitCode != 0) {
        printf("Failed to execute %d, exitcode %d\n", GetLastError(), ExitCode);
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int wmain(int, wchar_t**)
{
    if (GetBackupFile() == FALSE) {
        printf("Failed to get backup file\n");
        return 1;
    }
    if (BackupClassKeys() == FALSE) {
        printf("Exec failed\n");
        return 1;
    }

    EnumRegistryKey(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\ControlSet001\\Control\\Class",
        KEY_QUERY_VALUE,
        {},
        [](std::wstring key) {
            // skip usb
            if (EndsWith(key, L"{36fc9e60-c465-11cf-8056-444553540000}")) {
                return;
            }

            EnumRegistryValue(
                HKEY_LOCAL_MACHINE,
                key,
                KEY_WRITE,
                { REG_MULTI_SZ },
                { L"UpperFilters" },
                [&](int typeIndex, int nameIndex, std::wstring name, byte* value, DWORD size) {
                    UNREFERENCED_PARAMETER(typeIndex);
                    UNREFERENCED_PARAMETER(nameIndex);
                    std::list<std::wstring> filter1;
                    std::vector<WCHAR> data1(size / sizeof(WCHAR));
                    memcpy(&data1[0], value, size);
                    buildStringListFromVector(filter1, data1);

                    std::list<std::wstring> filter2;
                    buildNewListWithoutElement(filter2, filter1, L"VEDRDvFlt");

                    std::vector<WCHAR> data2;
                    buildMultiStringVectorFromList(data2, filter2);

                    RegSetKeyValueW(
                        HKEY_LOCAL_MACHINE,
                        key.c_str(),
                        name.c_str(),
                        REG_MULTI_SZ,
                        data2.data(),
                        data2.size() * sizeof(WCHAR)
                    );
                }
            );
        }
    );

    //
    bool success = true;
    EnumRegistryKey(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\ControlSet001\\Control\\Class",
        KEY_QUERY_VALUE,
        {},
        [&](std::wstring key) {
            // skip usb
            if (EndsWith(key, L"{36fc9e60-c465-11cf-8056-444553540000}")) {
                return;
            }

            EnumRegistryValue(
                HKEY_LOCAL_MACHINE,
                key,
                KEY_READ,
                { REG_MULTI_SZ },
                { L"UpperFilters" },
                [&](int typeIndex, int nameIndex, std::wstring name, byte* value, DWORD size) {
                    UNREFERENCED_PARAMETER(typeIndex);
                    UNREFERENCED_PARAMETER(nameIndex);
                    std::list<std::wstring> filter;
                    std::vector<WCHAR> data(size / sizeof(WCHAR));
                    memcpy(&data[0], value, size);
                    buildStringListFromVector(filter, data);

                    for (auto& devicename : filter) {
                        if (devicename == L"VEDRDvFlt") {
                            success = false;
                        }
                    }
                }
            );
        }
    );

    if (success) {
        printf("remove upperfilter success. Deleting backup\n");
        if (!DeleteFileW(g_BackupFile.c_str())) {
            printf("Delete file %ws failed %d\n", g_BackupFile.c_str(), GetLastError());
        }
    }
    else {
        printf("remove upperfilter failed\n");
        RestoreClassKeys();
    }
    return 0;
}
