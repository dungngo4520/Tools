#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>

using namespace std;

int wmain(int argc, wchar_t** argv)
{
    int n = 0;
    bool writedata = 0;
    int msleep = 0;
    wstring prefix = argv[0];
    prefix = prefix.substr(0, prefix.find_last_of(L'.'));

    if (argc == 1) {
        wcout << L"Usage " << argv[0] << L" <count> [writedata] [msleep] [prefix]" << endl;
        cout << "\tcount :number of files to write" << endl;
        cout << "\twritedata: indicate program should write random data to the file, true if != 0" << endl;
        cout << "\tmsleep: time to sleep between each write" << endl;
        cout << "\tprefix: file prefix, default is this program name" << endl;
        return 0;
    }

    int i = 1;
    if (argc >= i + 1) n = _wtoi(argv[i]);
    i++;

    if (argc >= i + 1) writedata = _wtoi(argv[i]) != 0;
    i++;

    if (argc >= i + 1) msleep = _wtoi(argv[i]);
    i++;

    if (argc >= i + 1) prefix = argv[i];
    i++;

    srand((unsigned int)chrono::system_clock::now().time_since_epoch().count());

    for (i = 0; i < n; i++) {
        wstring filename = prefix + L"_" + to_wstring(i);
        try {
            ofstream f(filename);
            if (writedata) {
                auto count = rand();
                auto buffer = unique_ptr<byte>(new byte[count]);
                for (int j = 0; j < count; j++) {
                    memset(buffer.get() + j, rand(), 1);
                }
                f.write((char*)buffer.get(), count);
            }
            wcout << L"Write file " << filename << L" ok" << endl;
        }
        catch (const std::exception& e) {
            wcout << L"Failed to write file " << filename << "L. ";
            cout << "Error " << e.what() << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(msleep));
    }
    return 0;
}
