#include "../Utils/windows/WindowsNtdll.h"
#include "../Utils/windows/WindowsUserUtils.h"
#include "../Utils/memory/MemoryUtils.h"
#include "../Utils/cli/CLI11.hpp"

#include <memory>
#include <ntstatus.h>

int wmain(int argc, wchar_t** argv)
{
    DWORD pid = 0;

    CLI::App app("List process handle");
    app.add_option("--pid", pid, "Target process id")->required();
    CLI11_PARSE(app, argc, argv);

    if (!utils::windows::user::IsElevated()) {
        printf("Rerun as administrator!\n");
        return 1;
    }

    if (!utils::windows::user::SetPrivilege(SE_DEBUG_NAME, true)) {
        printf("Failed to gain privileges\n");
        return 1;
    }

    if (utils::windows::ntdll::Load() != 0) {
        printf("Load library failed\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open target process\n");
        return 1;
    }
    defer { CloseHandle(hProcess); };

    size_t SystemHandleBufferSize = 0x10000;
    std::unique_ptr<byte[]> SystemHandleBuffer(new byte[SystemHandleBufferSize]);
    NTSTATUS status = STATUS_SUCCESS;
    while ((status = utils::windows::ntdll::QuerySystemInformation(
                utils::windows::ntdll::SystemHandleInformation,  // SystemInformationClass
                SystemHandleBuffer.get(),                        //SystemInformation
                SystemHandleBufferSize,                          //SystemInformationLength
                NULL                                             //ReturnLength
            )) == STATUS_INFO_LENGTH_MISMATCH) {
        SystemHandleBufferSize *= 2;
        SystemHandleBuffer.reset(new byte[SystemHandleBufferSize]);
    }
    if (!NT_SUCCESS(status)) {
        printf("Failed to query system handle\n");
        return 1;
    }

    auto SystemHandleInfo =
        reinterpret_cast<utils::windows::ntdll::SYSTEM_HANDLE_INFORMATION*>(SystemHandleBuffer.get());
    for (uint32_t i = 0, c = 0; i < SystemHandleInfo->HandleCount; i++) {
        if (SystemHandleInfo->Handles[i].ProcessId != pid) continue;

        HANDLE DupHandle = NULL;
        status = utils::windows::ntdll::DuplicateObject(
            hProcess,                                     // SourceProcessHandle
            (HANDLE)SystemHandleInfo->Handles[i].Handle,  // SourceHandle
            GetCurrentProcess(),                          // TargetProcessHandle
            &DupHandle,                                   // TargetHandle
            0,                                            // DesiredAccess
            0,                                            // Attributes
            0                                             //Options
        );
        if (!NT_SUCCESS(status)) continue;
        defer { CloseHandle(DupHandle); };

        size_t ObjectBufferSize = 1024;
        std::unique_ptr<byte[]> ObjectBuffer(new byte[1024]);
        while ((status = utils::windows::ntdll::QueryObject(
                    DupHandle, utils::windows::ntdll::ObjectTypeInformation, ObjectBuffer.get(), ObjectBufferSize, NULL
                )) == STATUS_INFO_LENGTH_MISMATCH) {
            ObjectBufferSize *= 2;
            ObjectBuffer.reset(new byte[ObjectBufferSize]);
        }
        if (!NT_SUCCESS(status)) continue;

        auto ObjectTypeInfo = reinterpret_cast<utils::windows::ntdll::OBJECT_TYPE_INFORMATION*>(ObjectBuffer.get());
        std::wstring output = ObjectTypeInfo->Name.Buffer;

        while ((status = utils::windows::ntdll::QueryObject(
                    DupHandle, utils::windows::ntdll::ObjectNameInformation, ObjectBuffer.get(), ObjectBufferSize, NULL
                )) == STATUS_INFO_LENGTH_MISMATCH) {
            ObjectBufferSize *= 2;
            ObjectBuffer.reset(new byte[ObjectBufferSize]);
        }
        output += L" - ";
        auto ObjectNameInfo = reinterpret_cast<utils::windows::ntdll::UNICODE_STRING*>(ObjectBuffer.get());
        if (!NT_SUCCESS(status) || ObjectNameInfo->Length == 0)
            output += L"Unknown";
        else
            output += std::wstring(ObjectNameInfo->Buffer, ObjectNameInfo->Length / 2);
        printf("[%d]\t%ls\n", ++c, output.c_str());
    }
    return 0;
}
