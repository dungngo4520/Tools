#include <Windows.h>
#include <iostream>

using namespace std;

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS2)(HANDLE, USHORT*, USHORT*);

int wmain(int, wchar_t**)
{
    SYSTEM_INFO sysInfo = {};
    GetNativeSystemInfo(&sysInfo);

    cout << "dwOemId " << sysInfo.dwOemId << endl;
    cout << "wProcessorArchitecture " << sysInfo.wProcessorArchitecture << endl;
    cout << "dwPageSize " << sysInfo.dwPageSize << endl;
    cout << "lpMinimumApplicationAddress " << hex << sysInfo.lpMinimumApplicationAddress << dec << endl;
    cout << "lpMaximumApplicationAddress " << hex << sysInfo.lpMaximumApplicationAddress << dec << endl;
    cout << "dwActiveProcessorMask " << sysInfo.dwActiveProcessorMask << endl;
    cout << "dwNumberOfProcessors " << sysInfo.dwNumberOfProcessors << endl;
    cout << "dwProcessorType " << sysInfo.dwProcessorType << endl;
    cout << "dwAllocationGranularity " << sysInfo.dwAllocationGranularity << endl;
    cout << "wProcessorLevel " << sysInfo.wProcessorLevel << endl;
    cout << "wProcessorRevision " << sysInfo.wProcessorRevision << endl;

    cout << endl;

    BOOL Wow64 = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &Wow64)) {
        cout << "WOW64: " << (Wow64 ? "true" : "false") << endl;
    }
    else {
        cout << "IsWow64Process failed " << GetLastError() << endl;
    }

    cout << endl;

    USHORT ProcessMachine = 0;
    USHORT NativeMachine = 0;
    LPFN_ISWOW64PROCESS2 pIsWow64Process2 =
        (LPFN_ISWOW64PROCESS2)GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process2");
    if (!pIsWow64Process2) {
        cout << "Failed to get address" << endl;
        return 1;
    }

    if (!pIsWow64Process2(GetCurrentProcess(), &ProcessMachine, &NativeMachine)) {
        cout << "IsWow64Process2 failed " << GetLastError() << endl;
        return 1;
    }

    cout << "ProcessMachine " << ProcessMachine << endl;
    cout << "NativeMachine " << NativeMachine << endl;

    cout << endl;

    system("pause");
    return 0;
}
