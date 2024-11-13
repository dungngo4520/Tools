#include <Windows.h>
#include <iostream>

using namespace std;

#include <wlanapi.h>
#pragma comment(lib, "Wlanapi.lib")

int wmain(int, wchar_t**)
{
    HANDLE hClient = NULL;
    DWORD ret = 0;
    DWORD dwNegotiatedVersion = 0;

    ret = WlanOpenHandle(2, NULL, &dwNegotiatedVersion, &hClient);
    if (ret != ERROR_SUCCESS) {
        cout << "Failed to open handle " << ret << endl;
        return ret;
    }


    DOT11_NETWORK_LIST list = {};
    list.dwIndex = 0;
    list.dwNumberOfItems = 0;
    list.Network->dot11BssType = dot11_BSS_type_any;
    list.Network->dot11Ssid.uSSIDLength = 0;

    ret = WlanSetFilterList(hClient, wlan_filter_list_type_user_deny, &list, NULL);
    if (ret != ERROR_SUCCESS) {
        cout << "Failed to set filter " << ret << endl;
    }

    WlanCloseHandle(hClient, NULL);

    return 0;
}
