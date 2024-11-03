#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;


int wmain(int argc, wchar_t** argv)
{
    if (argc < 2) {
        wcout << argv[0] << L" Usage: " << argv[0] << L" <servicename>" << endl;
        wcout << L"check if service is marked for deletion or not" << endl;
    }

    wstring servicename = argv[1];
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hSerivce = NULL;
    DWORD LastError = 0;

    hScManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hScManager) {
        cout << "Failed to open service manager" << GetLastError() << endl;
        goto Exit;
    }

    hSerivce = OpenServiceW(hScManager, servicename.c_str(), SERVICE_CHANGE_CONFIG);
    if (!hSerivce) {
        cout << "Failed to open service " << GetLastError() << endl;
        goto Exit;
    }

    ChangeServiceConfigW(
        hSerivce, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    );
    LastError = GetLastError();
    if (LastError == ERROR_SERVICE_MARKED_FOR_DELETE) {
        cout << "Service is marked for deletion" << endl;
    }
    else {
        cout << "Service is not marked delete " << LastError << endl;
    }

Exit:
    if (hScManager) CloseServiceHandle(hScManager);
    return 0;
}
