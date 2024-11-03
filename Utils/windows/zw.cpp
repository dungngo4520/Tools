#include "zw.h"

#include <ntstatus.h>

#include <assert.h>
#include <stdio.h>

using ZwQuerySystemInformationPtr = NTSTATUS(WINAPI *)(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Inout_ PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
);
using ZwQueryObjectPtr = NTSTATUS(WINAPI *)(
    _In_opt_ HANDLE Handle,
    _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
    _Out_opt_ PVOID ObjectInformation,
    _In_ ULONG ObjectInformationLength,
    _Out_opt_ PULONG ReturnLength
);

HMODULE libNTDLL = NULL;
ZwQuerySystemInformationPtr ZwQuerySystemInformation = NULL;
ZwQueryObjectPtr ZwQueryObject = NULL;

NTSTATUS library::ntdll::Load()
{
    if (libNTDLL != NULL) {
        printf(__FUNCTION__ ": Library ntdll.dll already loaded\n");
        return STATUS_SUCCESS;
    }

    libNTDLL = LoadLibraryExW(L"ntdll.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (libNTDLL == NULL) {
        printf(__FUNCTION__ ": Failed to load library ntdll.dll. Error: %d\n", GetLastError());
        return STATUS_UNSUCCESSFUL;
    }

    ZwQuerySystemInformation = (ZwQuerySystemInformationPtr)GetProcAddress(libNTDLL, "ZwQuerySystemInformation");
    ZwQueryObject = (ZwQueryObjectPtr)GetProcAddress(libNTDLL, "ZwQueryObject");
    return STATUS_SUCCESS;
}

NTSTATUS library::ntdll::zw::QuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
)
{
    assert(ZwQuerySystemInformation != NULL);
    return ZwQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
}

NTSTATUS library::ntdll::zw::QueryObject(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
)
{
    assert(ZwQueryObject != NULL);
    return ZwQueryObject(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
}
