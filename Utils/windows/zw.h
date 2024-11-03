#pragma once
#define UMDF_USING_NTSTATUS
#include <Windows.h>
#include <winternl.h>

namespace library {
    namespace ntdll {
        NTSTATUS Load();
        namespace zw {
            NTSTATUS QuerySystemInformation(
                SYSTEM_INFORMATION_CLASS SystemInformationClass,
                PVOID SystemInformation,
                ULONG SystemInformationLength,
                PULONG ReturnLength
            );
            NTSTATUS QueryObject(
                HANDLE Handle,
                OBJECT_INFORMATION_CLASS ObjectInformationClass,
                PVOID ObjectInformation,
                ULONG ObjectInformationLength,
                PULONG ReturnLength
            );
        };  // namespace zw
    };  // namespace ntdll
};  // namespace library
