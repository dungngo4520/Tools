#include "WindowsNtdll.h"

#include <ntstatus.h>

#include <assert.h>
#include <stdio.h>

#include <map>
#include <string>

using NtQuerySystemInformationPtr = NTSTATUS(NTAPI *)(
    _In_ utils::windows::ntdll::SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Inout_ PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
);
using NtQueryObjectPtr = NTSTATUS(NTAPI *)(
    _In_opt_ HANDLE Handle,
    _In_ utils::windows::ntdll::OBJECT_INFORMATION_CLASS ObjectInformationClass,
    _Out_opt_ PVOID ObjectInformation,
    _In_ ULONG ObjectInformationLength,
    _Out_opt_ PULONG ReturnLength
);
using NtDuplicateObjectPtr = NTSTATUS(NTAPI *)(
    _In_ HANDLE SourceProcessHandle,
    _In_ HANDLE SourceHandle,
    _In_opt_ HANDLE TargetProcessHandle,
    _Out_opt_ PHANDLE TargetHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ ULONG HandleAttributes,
    _In_ ULONG Options
);

HMODULE libNTDLL = NULL;
std::map<std::string, FARPROC> FunctionMap;

// NtQuerySystemInformationPtr NtQuerySystemInformation = NULL;
// NtDuplicateObjectPtr NtDuplicateObject = NULL;
// NtQueryObjectPtr NtQueryObject = NULL;

NTSTATUS utils::windows::ntdll::Load()
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

    FunctionMap["NtQuerySystemInformation"] = GetProcAddress(libNTDLL, "NtQuerySystemInformation");
    FunctionMap["NtDuplicateObject"] = GetProcAddress(libNTDLL, "NtDuplicateObject");
    FunctionMap["NtQueryObject"] = GetProcAddress(libNTDLL, "NtQueryObject");
    return STATUS_SUCCESS;
}

NTSTATUS utils::windows::ntdll::QuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
)
{
    assert(FunctionMap["NtQuerySystemInformation"] != NULL);
    return ((NtQuerySystemInformationPtr)FunctionMap["NtQuerySystemInformation"])(
        SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength
    );
}

NTSTATUS utils::windows::ntdll::DuplicateObject(
    HANDLE SourceProcessHandle,
    HANDLE SourceHandle,
    HANDLE TargetProcessHandle,
    PHANDLE TargetHandle,
    ACCESS_MASK DesiredAccess,
    ULONG Attributes,
    ULONG Options
)
{
    assert(FunctionMap["NtDuplicateObject"] != NULL);
    return ((NtDuplicateObjectPtr)FunctionMap["NtDuplicateObject"])(
        SourceProcessHandle, SourceHandle, TargetProcessHandle, TargetHandle, DesiredAccess, Attributes, Options
    );
}

NTSTATUS utils::windows::ntdll::QueryObject(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
)
{
    assert(FunctionMap["NtQueryObject"] != NULL);
    return ((NtQueryObjectPtr)FunctionMap["NtQueryObject"])(
        Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength
    );
}
