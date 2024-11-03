#include < Windows.h>
#include <ntstatus.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ShlObj.h>
#include <wuapi.h>
#include <wuerror.h>
#include <atlbase.h>
#include <ATLComTime.h>
#include <propkey.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")
#pragma comment(lib, "Advapi32.lib")

using namespace std;

template <typename Func>
LSTATUS
    EnumRegistryKey(HKEY RootKey, const wstring& KeyPath, const DWORD& Flags, const set<wstring>& Filters, Func func)
{
    HKEY hKey = NULL;
    LSTATUS status = ERROR_SUCCESS;
    DWORD Index = 0;

    DWORD NameBufferSize = 255;  // Max name length
    unique_ptr<wchar_t> NameBuffer = unique_ptr<wchar_t>(new wchar_t[NameBufferSize]);

    if (!NameBuffer.get()) return ERROR_INSUFFICIENT_BUFFER;

    status = RegOpenKeyExW(RootKey, KeyPath.c_str(), 0, KEY_READ | Flags, &hKey);
    if (status != ERROR_SUCCESS) return status;

    auto TempNameBufferSize = NameBufferSize;
    while (status == ERROR_SUCCESS) {
        status = RegEnumKeyExW(hKey, Index, NameBuffer.get(), &TempNameBufferSize, NULL, NULL, NULL, NULL);
        if (status != ERROR_SUCCESS) break;

        wstring SubKeyName = NameBuffer.get();
        bool callback = Filters.empty();

        for (auto& filter : Filters) {
            if (SubKeyName.find(filter) != wstring::npos) {
                callback = true;
                break;
            }
        }

        if (callback) func(SubKeyName);

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
    const wstring& KeyPath,
    const DWORD& Flags,
    const vector<DWORD>& TypeFilters,
    const vector<wstring>& NameFilters,
    Func func
)
{
    HKEY hKey = NULL;
    LSTATUS status = ERROR_SUCCESS;
    DWORD Index = 0;

    DWORD NameBufferSize = 32767;  // Max name length
    unique_ptr<wchar_t> Buffer = unique_ptr<wchar_t>(new wchar_t[NameBufferSize]);
    DWORD Type = 0;
    DWORD ValueBufferSize = 4096;
    unique_ptr<byte> ValueBuffer = unique_ptr<byte>(new byte[ValueBufferSize]);

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

        wstring ValueName = Buffer.get();
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
            if (ValueName.find(filter) != wstring::npos) {
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

typedef struct ProductInfo {
    wstring Name;
    wstring Version;
    wstring Publisher;
    wstring InstallDate;
} ProductInfo;
bool operator<(const ProductInfo& l, const ProductInfo& r) { return l.Name.compare(r.Name) < 0; }

enum class Arch { X86, X64 };
set<ProductInfo> GetProductList(Arch arch)
{
    set<ProductInfo> InfoList;
    wstring RootPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    DWORD Flags = 0;

    switch (arch) {
        case Arch::X86:
            Flags |= KEY_WOW64_32KEY;
            break;
        case Arch::X64:
            Flags |= KEY_WOW64_64KEY;
            break;
        default:
            break;
    }

    LSTATUS status = EnumRegistryKey(HKEY_LOCAL_MACHINE, RootPath.c_str(), Flags, {}, [&](const wstring& SubKeyName) {
        //wcout << SubKeyName << endl;
        wstring SubKeyPath = RootPath + L"\\" + SubKeyName;
        ProductInfo Info = {};
        bool Found = false;

        LSTATUS status1 = EnumRegistryValue(
            HKEY_LOCAL_MACHINE,
            SubKeyPath,
            Flags,
            { REG_SZ },
            { L"DisplayName", L"DisplayVersion", L"Publisher", L"InstallDate" },
            [&](int TypeIndex, int NameIndex, wstring ValueName, PBYTE Value, DWORD ValueLength) {
                UNREFERENCED_PARAMETER(TypeIndex);
                switch (NameIndex) {
                    case 0:
                        Info.Name = wstring((wchar_t*)Value, ValueLength / sizeof(wchar_t));
                        Found = true;
                        break;
                    case 1:
                        Info.Version = wstring((wchar_t*)Value, ValueLength / sizeof(wchar_t));
                        break;
                    case 2:
                        Info.Publisher = wstring((wchar_t*)Value, ValueLength / sizeof(wchar_t));
                        break;
                    case 3:
                        Info.InstallDate = wstring((wchar_t*)Value, ValueLength / sizeof(wchar_t));
                        break;
                    default:
                        break;
                }
            }
        );

        if (status1 != ERROR_SUCCESS) {
            wcout << L"Failed to enum key " << SubKeyPath << L" Error " << hex << status1 << dec << endl;
            return;
        }

        if (Found) InfoList.emplace(Info);
    });

    if (status != ERROR_SUCCESS) {
        wcout << L"Failed to enum key " << RootPath << L" Error " << hex << status << dec << endl;
    }
    return InfoList;
}

typedef struct {
    DWORD MajorVersion;
    DWORD MinorVersion;
    DWORD BuildNumber;
    DWORD ProductType;
    DWORD ProductMask;
} OSVersionInfo;
#pragma warning(push)
#pragma warning(disable : 4996 28159)
OSVersionInfo GetOsVersion()
{
    OSVersionInfo ret;
    OSVERSIONINFOEXW Info;

    Info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((LPOSVERSIONINFOW)&Info);
    ret.MajorVersion = Info.dwMajorVersion;
    ret.MinorVersion = Info.dwMinorVersion;
    ret.BuildNumber = Info.dwBuildNumber;
    ret.ProductType = Info.wProductType;
    ret.ProductMask = Info.wSuiteMask;

    return ret;
}
#pragma warning(pop)

int GetUpdatePacks()
{
    HRESULT res;
    int ret = 1;

    res = CoInitializeEx(NULL, 0);
    if (SUCCEEDED(res)) {
        CComPtr<IUpdateSession> updateSession;
        CComPtr<IUpdateSearcher> updateSearcher;
        CComPtr<ISearchResult> searchResult;
        CComPtr<IUpdateCollection> updates;
        CComPtr<IUpdateHistoryEntryCollection> histories;

        res = CoCreateInstance(
            CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&updateSession
        );
        if (FAILED(res)) {
            printf("Failed to create update session %x\n", res);
            goto cleanup;
        }

        res = updateSession->CreateUpdateSearcher(&updateSearcher);
        if (FAILED(res)) {
            printf("Failed to create update searcher %x\n", res);
            goto cleanup;
        }

        res = updateSearcher->put_ServerSelection(ssWindowsUpdate);
        if (FAILED(res)) {
            printf("Failed to set server options %x\n", res);
            goto cleanup;
        }

        res = updateSearcher->put_Online(VARIANT_FALSE);
        if (FAILED(res)) {
            printf("Failed to set online options %x\n", res);
            goto cleanup;
        }

        cout << "Searching for updates..." << endl;

        BSTR criteria = SysAllocString(L"IsInstalled=1");
        res = updateSearcher->Search(criteria, &searchResult);
        SysFreeString(criteria);
        if (FAILED(res)) {
            printf("Failed to search for updates %x\n", res);
            goto cleanup;
        }

        res = searchResult->get_Updates(&updates);
        if (FAILED(res)) {
            printf("Failed to retrieve update list from search result %x\n", res);
            goto cleanup;
        }

        LONG updateCount;
        res = updates->get_Count(&updateCount);
        if (FAILED(res)) {
            printf("Failed to get update count %x\n", res);
            goto cleanup;
        }

        for (LONG i = 0L; i < updateCount; ++i) {
            CComPtr<IUpdate> update;
            CComPtr<IStringCollection> updateKBIDs;

            res = updates->get_Item(i, &update);
            if (FAILED(res)) {
                printf("Failed to get update item %x\n", res);
                goto cleanup;
            }

            res = update->get_KBArticleIDs(&updateKBIDs);
            if (FAILED(res)) {
                printf("Failed to get KB article ID list %x\n", res);
                goto cleanup;
            }

            LONG kbIDCount;
            res = updateKBIDs->get_Count(&kbIDCount);
            if (FAILED(res)) {
                printf("Failed to get KB article ID count %x\n", res);
                goto cleanup;
            }

            for (LONG j = 0L; j < kbIDCount; ++j) {
                BSTR kbID;
                res = updateKBIDs->get_Item(j, &kbID);
                if (FAILED(res)) {
                    printf("Failed to get KB article ID %x\n", res);
                    goto cleanup;
                }
                std::wcout << L"KB" << kbID << L':' << std::endl;
                SysFreeString(kbID);
            }

            BSTR updateTitle;
            res = update->get_Title(&updateTitle);
            if (FAILED(res)) {
                printf("Failed to get update updateTitle %x\n", res);
                goto cleanup;
            }
            std::wcout << L"  " << updateTitle << std::endl << std::endl;
            SysFreeString(updateTitle);
        }

        printf("\nTotal %d updates\n", updateCount);


        //LONG total = 0;

        //res = updateSearcher->GetTotalHistoryCount(&total);
        //if (FAILED(res)) {
        //	printf("Failed to get update history %x\n", res);
        //	goto cleanup;
        //}

        //res = updateSearcher->QueryHistory(0, total, &histories);
        //if (FAILED(res)) {
        //	printf("Failed to get query history %x\n", res);
        //	goto cleanup;
        //}

        //for (int i = 0; i < total; i++) {
        //	CComPtr<IUpdateHistoryEntry> entry;
        //	BSTR updateTitle;
        //	BSTR updateLink;
        //	DATE installedDate;

        //	res = histories->get_Item(i, &entry);
        //	if (FAILED(res)) {
        //		printf("Failed to get history entry %x\n", res);
        //		continue;
        //	}

        //	res = entry->get_Title(&updateTitle);
        //	if (FAILED(res)) {
        //		printf("Failed to get entry updateTitle %x\n", res);
        //		continue;
        //	}

        //	res = entry->get_Date(&installedDate);
        //	if (FAILED(res)) {
        //		printf("Failed to get entry updateTitle %x\n", res);
        //		continue;
        //	}

        //	res = entry->get_SupportUrl(&updateLink);
        //	if (FAILED(res)) {
        //		printf("Failed to get entry updateTitle %x\n", res);
        //		continue;
        //	}

        //	COleDateTime date(installedDate);

        //	printf("%d/%d/%d, %ws, %ws\n", date.GetDay(), date.GetMonth(), date.GetYear(), updateTitle, updateLink);

        //	SysFreeString(updateTitle);
        //	SysFreeString(updateLink);
        //}

        //printf("\nTotal %d updates\n", total);
    }

    ret = 0;

cleanup:
    CoUninitialize();
    return ret;
}

#include <Msi.h>
#pragma comment(lib, "Msi.lib")
int GetUpdatePack2()
{
    wchar_t patchCode[39] = {};
    wchar_t productCode[39] = {};
    int index;
    wchar_t buffer[1024] = {};
    DWORD buffersize = 1024;
    DWORD tempsize = buffersize;

    struct patches {
        wstring productname;
        wstring installdate;
    };
    vector<patches> list;

    for (index = 0;
         MsiEnumPatchesExW(
             NULL, NULL, MSIINSTALLCONTEXT_MACHINE, MSIPATCHSTATE_APPLIED, index, patchCode, productCode, NULL, NULL, NULL
         ) == ERROR_SUCCESS;
         index++) {
        tempsize = buffersize;
        wstring productName;
        wstring installedDate;

        if (ERROR_SUCCESS ==
            MsiGetPatchInfoExW(
                patchCode, productCode, NULL, MSIINSTALLCONTEXT_MACHINE, INSTALLPROPERTY_DISPLAYNAME, buffer, &tempsize
            )) {
            productName = buffer;
        }

        if (ERROR_SUCCESS ==
            MsiGetPatchInfoExW(
                patchCode, productCode, NULL, MSIINSTALLCONTEXT_MACHINE, INSTALLPROPERTY_INSTALLDATE, buffer, &tempsize
            )) {
            installedDate = buffer;
        }

        list.push_back({ productName, installedDate });
    }

    sort(list.begin(), list.end(), [&](const patches& l, const patches& r) { return l.installdate > r.installdate; });
    for (auto& p : list) {
        wcout << p.installdate << L" - " << p.productname << endl;
    }

    wcout << L"Total " << index << L" update" << endl;
    return 0;
}

#include <propvarutil.h>
#include <propsys.h>
#pragma comment(lib, "Propsys.lib")
int GetUpdatePack3()
{
    HRESULT hr = CoInitialize(NULL);
    int count = 0;

    if (SUCCEEDED(hr)) {
        CComPtr<IShellItem> pUpdates;
        CComPtr<IEnumShellItems> pShellEnum;

        hr = SHGetKnownFolderItem(FOLDERID_AppUpdates, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&pUpdates));
        if (FAILED(hr) || !pUpdates) {
            printf("Failed to get folder %d\n", hr);
            goto cleanup;
        }

        hr = pUpdates->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&pShellEnum));
        if (FAILED(hr) || !pShellEnum) {
            printf("Failed to get folder %d\n", hr);
            goto cleanup;
        }

        do {
            CComPtr<IShellItem> pItem;
            CComPtr<IPropertyStore> pStore;
            CComHeapPtr<WCHAR> szName;
            HRESULT hres = S_OK;

            hr = pShellEnum->Next(1, &pItem, nullptr);
            if (FAILED(hr) || !pItem) continue;

            hres = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &szName);
            if (FAILED(hres) || !szName) continue;

            wcout << static_cast<LPWSTR>(szName) << endl;
            count++;

            hres = pItem->BindToHandler(NULL, BHID_PropertyStore, IID_PPV_ARGS(&pStore));
            if (FAILED(hres) || !pStore) continue;

            DWORD ItemCount = 0;
            pStore->GetCount(&ItemCount);
            for (DWORD i = 0; i < ItemCount; i++) {
                PROPERTYKEY pk;
                if (SUCCEEDED(pStore->GetAt(i, &pk))) {
                    CComHeapPtr<WCHAR> pkName;
                    PSGetNameFromPropertyKey(pk, &pkName);

                    PROPVARIANT pv;
                    PropVariantInit(&pv);
                    if (SUCCEEDED(pStore->GetValue(pk, &pv))) {
                        CComHeapPtr<wchar_t> pvs;
                        pvs.Allocate(512);
                        PropVariantToString(pv, pvs, 512);  // needs propvarutil.h and propsys.lib
                        PropVariantClear(&pv);
                        printf(" %ls=%ls\n", pkName.m_pData, pvs.m_pData);
                    }
                }
            }
        } while (hr == S_OK);
    }

cleanup:
    CoUninitialize();
    wcout << L"Found " << count << " updates" << endl;
    return 0;
}


int wmain(int argc, wchar_t** argv)
{
    wofstream f;
    bool fileoutput = false;

    if (argc >= 2) {
        try {
            f.open(argv[1]);
        }
        catch (const std::exception& e) {
            cout << e.what() << endl;
            return 1;
        }
        fileoutput = true;
    }

    //wostream& out = fileoutput ? f : wcout;

    auto ProductList64 = GetProductList(Arch::X64);
    //auto ProductList32 = GetProductList(Arch::X86);

    //int widthName = 0, widthPublisher = 0, widthInstallDate = 0, widthVersion = 0;
    //for (auto& product : ProductList64) {
    //	if (widthName < product.Name.buffersize()) widthName = product.Name.buffersize();
    //	if (widthPublisher < product.Publisher.buffersize()) widthPublisher = product.Publisher.buffersize();
    //	if (widthInstallDate < product.InstallDate.buffersize()) widthInstallDate = product.InstallDate.buffersize();
    //	if (widthVersion < product.Version.buffersize()) widthVersion = product.Version.buffersize();
    //}
    //widthName += 2;
    //widthPublisher += 2;
    //widthInstallDate += 2;
    //widthVersion += 2;

    //out << endl << L"PRODUCTS 64bit" << endl;
    //for (auto& product : ProductList64) {
    //	wcout << setw(widthName) << left << product.Name;
    //	wcout << setw(widthPublisher) << left << product.Publisher;
    //	wcout << setw(widthInstallDate) << left << product.InstallDate;
    //	wcout << setw(widthVersion) << left << product.Version;
    //	wcout << endl;
    //}

    //out << endl << L"PRODUCTS 32bit" << endl;
    //for (auto& product : ProductList32) {
    // out << product.Name << L": " << product.Version << endl;
    //}

    //GetUpdatePacks();
    //GetUpdatePack2();
    //GetUpdatePack3();

    system("pause");
    return 0;
}
